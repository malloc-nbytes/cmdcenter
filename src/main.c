#include <forge/chooser.h>
#include <forge/cmd.h>
#include <forge/err.h>
#include <forge/arg.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define FLAG_2HY_KEYBOARD "keyboard"
#define FLAG_2HY_XSET "xset"

typedef enum {
        FT_KEYBOARD = 1 << 0,
        FT_XSET = 1 << 1,
} ft_type;

static struct {
        uint32_t flags;
} g_config = {
        .flags = 0x00000000,
};

void all(void);

void
fix_audio(void)
{
        if (!is_sudo()) {
                forge_err("This action requires sudo access");
        }
        cmd("echo 0 | sudo tee > /sys/module/snd_hda_intel/parameters/power_save");
        exit(0);
}

void
keyboard(void)
{
        int swap;

        if (g_config.flags & FT_XSET) {
                swap = 0;
        } else if (g_config.flags & FT_KEYBOARD) {
                swap = 1;
        } else {
                swap = forge_chooser_yesno("Would you like to swap CAPS with CTRL?", NULL, 1);
        }

        if (swap) {
                cmd("setxkbmap -option");
                cmd("xmodmap -e \"clear Lock\"");
                cmd("xmodmap -e \"keycode 66 = Control_L\"");

                int enter;
                if (g_config.flags & FT_KEYBOARD) {
                        enter = 1;
                } else {
                        enter = forge_chooser_yesno("Would you like to make LCTRL ENTER?", NULL, 1);
                }

                if (enter) {
                        cmd("xmodmap -e \"keycode 37 = Return\"");
                }

                cmd("xmodmap -e \"add Control = Control_L Control_R\"");
        }

        cmd("xset r rate 150 50");
}

void
arandr(void)
{
        cmd("arandr");
}

void
picom(void)
{
        cmd("picom -b");
}

void
animx(void)
{
        sleep(1);
        cmd("AnimX --restore");
}

void (*funs[])(void) = {
        all,
        fix_audio,
        keyboard,
        arandr,
        picom,
        animx,
};

void
all(void)
{
        for (size_t i = 2; i < sizeof(funs)/sizeof(*funs); ++i) {
                funs[i]();
        }
}

int
main(int argc, char **argv)
{
        const char *choices[] = {
                "PERFORM ALL (except fix audio [requires sudo])", // MUST BE HERE
                "FIX AUDIO POPPING",// MUST BE HERE
                "KEYBOARD SETUP",
                "ADJUST MONITOR LOCATIONS",
                "ENABLE COMPOSITOR",
                "LAUNCH BACKGROUND",
        };

        _Static_assert(sizeof(choices)/sizeof(*choices) == sizeof(funs)/sizeof(*funs));

        forge_arg *arghd = forge_arg_alloc(argc, argv, 1);
        forge_arg *arg = arghd;
        while (arg) {
                if (arg->h == 2 && !strcmp(arg->s, FLAG_2HY_KEYBOARD)) {
                        g_config.flags |= FT_KEYBOARD;
                } else if (arg->h == 2 && !strcmp(arg->s, FLAG_2HY_XSET)) {
                        g_config.flags |= FT_XSET;
                } else {
                        forge_err_wargs("unknown option `%s`", arg->s);
                }
                arg = arg->n;
        }
        forge_arg_free(arghd);

        if ((g_config.flags & FT_KEYBOARD) || (g_config.flags & FT_XSET)) {
                keyboard();
                exit(0);
        }

        while (1) {
                int choice = forge_chooser("CMDCENTER", choices, sizeof(choices)/sizeof(*choices), 0);
                if (choice == -1) break;
                funs[choice]();
                if (choice == 1) break;
        }

        return 0;
}

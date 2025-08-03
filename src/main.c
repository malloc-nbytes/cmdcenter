#include <forge/chooser.h>
#include <forge/cmd.h>
#include <forge/err.h>

#include <stdio.h>
#include <unistd.h>

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
        int swap = forge_chooser_yesno("Would you like to swap CAPS with CTRL?", NULL, 1);

        if (swap) {
                cmd("setxkbmap -option");
                cmd("xmodmap -e \"clear Lock\"");
                cmd("xmodmap -e \"keycode 66 = Control_L\"");

                int enter = forge_chooser_yesno("Would you like to make LCTRL ENTER?", NULL, 1);

                if (enter) {
                        cmd("xmodmap -e \"keycode 37 = Return\"");
                }

                cmd("xmodmap -e \"add Control = Control_L Control_R\"");
        }

        cmd("xset r rate 210 50");
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
main(void)
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

        int choice = forge_chooser("CMDCENTER", choices, sizeof(choices)/sizeof(*choices), 0);

        if (choice == -1) return 0;

        funs[choice]();

        return 0;
}

#include <SDL.h>
#include <vector>

#include "Window.h"
#include "Textures.h"
#include "Sound.h"
#include "Board.h"
#include "constants.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

Board * board;
Window * window;

bool running = false;

void handleInput(std::vector<Input> input) {
    for (Input i: input) {
        int any_ret;
        if (i == Input::CLOSE || (any_ret=board->AnyKey(i))==666) {
            running = false;
        } else if (any_ret) {
            switch (i)
            {
            case Input::RIGHT:
                board->Right(); break;
            case Input::LEFT:
                board->Left(); break;
            case Input::UP:
                board->Up(); break;
            case Input::DOWN:
                board->Down(); break;
            case Input::FIRE:
                board->Fire(); break;
            case Input::RESTART:
                board->Restart(); break;
            case Input::NEXTLEVEL:
                board->NextLevel(); break;
            case Input::PREVIOUSLEVEL:
                board->Previous(); break;
            case Input::SAVESTATE:
                board->Save();
                break;
            case Input::RESTORESTATE:
                board->Restore();
                break;
            case Input::UNDO:
                board->Undo();
                break;
            case Input::EXIT:
                running = false; break;
            case Input::HELPKEYS:
            case Input::HELPBLOCKS:
            case Input::ANY:
            case Input::NONE:
            case Input::CLOSE:
                // Not handled here
                break;
            }
        }
    }

}

// One iteration of what used to be the body of main()'s while(running)
// loop below - the actual per-frame logic is completely unchanged, just
// extracted into its own function so it can be handed to
// emscripten_set_main_loop() on the web build, which calls this once per
// browser animation frame (via requestAnimationFrame) instead of relying
// on a traditional blocking loop. That matters because this loop never
// calls anything blocking at all (no SDL_Delay, no SDL_WaitEvent) - which
// is exactly why Asyncify had nothing to correctly hook into and the
// game was silently stuck the moment main() started. Native builds
// (Linux/Windows/Haiku/Vita/PSP, see the plain while(running) loop in
// main() below) still just call this the exact same way as before this
// change - this refactor doesn't alter their behavior at all, only how
// the same per-frame work gets invoked on the web specifically.
void main_loop() {
    handleInput(window->getInput());
    board->Animate();
#ifdef __EMSCRIPTEN__
    if (!running) {
        emscripten_cancel_main_loop();
    }
#endif
}

int main(int, char**) {
    window = new Window(TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
    Draw * draw = new Draw(window->renderer);
    Textures * textures = new Textures(window->renderer);
    Sound * sound = new Sound();
    board = new Board(draw, textures, sound);

    running = true;

#ifdef __EMSCRIPTEN__
    // 0 fps = use the browser's own animation timing rather than a fixed
    // rate; simulate_infinite_loop=1 tells Emscripten this call itself
    // should never return control to the code below it, matching how the
    // native while(running) loop also never falls through until the game
    // actually exits.
    emscripten_set_main_loop(main_loop, 0, 1);
#else
    while (running) {
        main_loop();
    }
#endif

    delete textures;
    delete sound;
    delete board;
    delete window;

    return 0;
}

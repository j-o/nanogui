/*
    src/example2.cpp -- C++ version of an example application that shows
    how to use the form helper class. For a Python implementation, see
    '../python/example2.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/nanogui.h>
#include <iostream>
#include <thread>

using namespace nanogui;

enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

Screen *screen = nullptr;

int main(int /* argc */, char ** /* argv */) {
    glfwSetErrorCallback(
        [](int error, const char *descr) {
        if (error == GLFW_NOT_INITIALIZED)
            return; /* Ignore */
        std::cerr << "GLFW error " << error << ": " << descr << std::endl;
    }
    );

    if (!glfwInit())
        throw std::runtime_error("Could not initialize GLFW!");

    glfwSetTime(0);

    bool use_gl_4_1 = false;// Set to true to create an OpenGL 4.1 context.
        

    /* Request a forward compatible OpenGL glMajor.glMinor core profile context.
        Default value is an OpenGL 3.3 core profile context. */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 0);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    auto size = Vector2i(500, 700);
    auto mGLFWWindow = glfwCreateWindow(size.x(), size.y(), "test", nullptr, nullptr);
    glfwSetWindowSize(mGLFWWindow, size.x(), size.y());

    if (!mGLFWWindow)
        throw std::runtime_error("Could not create an OpenGL " +
                                    std::to_string(4) + "." +
                                    std::to_string(1) + " context!");

    glfwMakeContextCurrent(mGLFWWindow);

#if defined(NANOGUI_GLAD)
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        throw std::runtime_error("Could not initialize GLAD!");
    glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM
#endif

    auto mBackground = nanogui::Color(0.3f, 0.3f, 0.32f, 1.f);

    auto mFBSize = Eigen::Vector2i();
    glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
    glViewport(0, 0, mFBSize[0], mFBSize[1]);
    glClearColor(mBackground[0], mBackground[1], mBackground[2], mBackground[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glfwSwapInterval(0);
    glfwSwapBuffers(mGLFWWindow);

    // NanoGUI presents many options for you to utilize at your discretion.
    // See include/nanogui/screen.h for what all of these represent.
    screen = new Screen( "NanoGUI test [GL 4.1]");

#if defined(__APPLE__)
    /* Poll for events once before starting a potentially
        lengthy loading process. This is needed to be
        classified as "interactive" by other software such
        as iTerm2 */

    glfwPollEvents();
#endif

    /* Propagate GLFW events to the appropriate Screen instance */
    glfwSetCursorPosCallback(mGLFWWindow,
        [](GLFWwindow *w, double x, double y) {
            screen->cursorPosCallbackEvent(x, y);
        }
    );

    glfwSetMouseButtonCallback(mGLFWWindow,
        [](GLFWwindow *w, int button, int action, int modifiers) {
            screen->mouseButtonCallbackEvent(button, action, modifiers);
        }
    );

    glfwSetKeyCallback(mGLFWWindow,
        [](GLFWwindow *w, int key, int scancode, int action, int mods) {
            screen->keyCallbackEvent(key, scancode, action, mods);
        }
    );

    glfwSetCharCallback(mGLFWWindow,
        [](GLFWwindow *w, unsigned int codepoint) {
            screen->charCallbackEvent(codepoint);
        }
    );

    glfwSetDropCallback(mGLFWWindow,
        [](GLFWwindow *w, int count, const char **filenames) {
            screen->dropCallbackEvent(count, filenames);
        }
    );

    glfwSetScrollCallback(mGLFWWindow,
        [](GLFWwindow *w, double x, double y) {
            screen->scrollCallbackEvent(x, y);
        }
    );

    /* React to framebuffer size events -- includes window
        size events and also catches things like dragging
        a window from a Retina-capable screen to a normal
        screen on Mac OS X */
    glfwSetFramebufferSizeCallback(mGLFWWindow,
        [](GLFWwindow* w, int width, int height) {
            screen->resizeCallbackEvent(width, height);
        }
    );

    bool enabled = true;
    FormHelper *gui = new FormHelper(screen);
    ref<Window> window = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar);
    gui->addVariable("string", strval);

    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar);
    gui->addVariable("double", dvar)->setSpinnable(true);

    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)
        ->setItems({"Item 1", "Item 2", "Item 3"});
    gui->addVariable("Color", colval)
        ->setFinalCallback([](const Color &c) {
                std::cout << "ColorPicker Final Callback: ["
                        << c.r() << ", "
                        << c.g() << ", "
                        << c.b() << ", "
                        << c.w() << "]" << std::endl;
            });

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; });

    screen->setVisible(true);
    screen->performLayout();
    window->center();

    auto refresh = 50;
    auto mainloop_active = true;
    std::thread refresh_thread;
    if (refresh > 0) {
        /* If there are no mouse/keyboard events, try to refresh the
        view roughly every 50 ms (default); this is to support animations
        such as progress bars while keeping the system load
        reasonably low */
        refresh_thread = std::thread(
            [refresh, &mainloop_active]() {
            std::chrono::milliseconds time(refresh);
            while (mainloop_active) {
                std::this_thread::sleep_for(time);
                glfwPostEmptyEvent();
            }
        }
        );
    }

    try {
        while (mainloop_active) {
            int numScreens = 0;

            if (!screen->visible()) {
                continue;
            } else if (glfwWindowShouldClose(mGLFWWindow)) {
                screen->setVisible(false);
                continue;
            }

            glfwMakeContextCurrent(mGLFWWindow);

            //auto mFBSize = Eigen::Vector2i{};

            //glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
            //glfwGetWindowSize(mGLFWWindow, &mSize[0], &mSize[1]);

            screen->drawAll();
            numScreens++;

            if (numScreens == 0) {
                /* Give up if there was nothing to draw */
                mainloop_active = false;
                break;
            }

            glfwSwapBuffers(mGLFWWindow);

            /* Wait for mouse/keyboard or empty refresh events */
            glfwWaitEvents();
        }

        /* Process events once more */
        glfwPollEvents();
    } catch (const std::exception &e) {
        std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
        leave();
    }

    if (refresh > 0)
        refresh_thread.join();

    nanogui::shutdown();
    return 0;
}

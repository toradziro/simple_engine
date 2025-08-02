#include "window_system.h"

#include <application/core/event_interface.h>
#include <application/managers/window_manager.h>
#include <application/core/event_interface.h>
#include <application/engine_context.h>
#include <application/core/utils/engine_assert.h>

//-------------------------------------------------------------------------------------------------
WindowSystem::WindowSystem(std::shared_ptr<EngineContext> context, WindowInfo&& info) noexcept
    : m_context(context)
    , m_info(std::move(info))
{
    auto res = glfwInit();
    engineAssert(res, "Glfw was not able to init");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    auto& winManager = m_context->m_managerHolder.getManager<WindowManager>();
    winManager.m_window = glfwCreateWindow(m_info.m_width
        , m_info.m_height
        , m_info.m_windowName.c_str()
        , nullptr
        , nullptr);

    glfwSetWindowUserPointer(winManager.m_window, &m_info);

    glfwSetWindowCloseCallback(winManager.m_window, [](GLFWwindow* window)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);

            WindowCloseEvent event;
            Event wrappedEvent(std::move(event));

            winInfo->m_eventCallback(wrappedEvent);
        });

    glfwSetWindowSizeCallback(winManager.m_window, [](GLFWwindow* window, int width, int height)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);

            winInfo->m_width = width;
            winInfo->m_height = height;

            WindowResizeEvent event{
                .m_width = width,
                .m_height = height
            };
            Event wrappedEvent(std::move(event));

            winInfo->m_eventCallback(wrappedEvent);
        });

    glfwSetKeyCallback(winManager.m_window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                KeyPressedEvent event{
                    .m_keyCode = key,
                    .m_repeatCount = 0
                };
                Event wrappedEvent(std::move(event));

                winInfo->m_eventCallback(wrappedEvent);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event{ .m_keyCode = key };
                Event wrappedEvent(std::move(event));

                winInfo->m_eventCallback(wrappedEvent);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event{
                    .m_keyCode = key,
                    .m_repeatCount = 1
                };
                Event wrappedEvent(std::move(event));

                winInfo->m_eventCallback(wrappedEvent);
                break;
            }
            default:
                break;
            }
        });

    glfwSetMouseButtonCallback(winManager.m_window, [](GLFWwindow* window, int button, int action, int /*mods*/)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);
            switch (action)
            {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event{ .m_buttonCode = button };
                Event wrappedEvent(std::move(event));

                winInfo->m_eventCallback(wrappedEvent);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event{ .m_buttonCode = button };
                Event wrappedEvent(std::move(event));

                winInfo->m_eventCallback(wrappedEvent);
                break;
            }
            default:
                break;
            }
        });

    glfwSetScrollCallback(winManager.m_window, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent event{
                .m_offsetX = (float)xoffset,
                .m_offsetY = (float)yoffset
            };
            Event wrappedEvent(std::move(event));

            winInfo->m_eventCallback(wrappedEvent);
        });

    glfwSetCursorPosCallback(winManager.m_window, [](GLFWwindow* window, double xpos, double ypos)
        {
            WindowInfo* winInfo = (WindowInfo*)glfwGetWindowUserPointer(window);
            MouseMovedEvent event{
                .m_mouseX = (float)xpos,
                .m_mouseY = (float)ypos
            };
            Event wrappedEvent(std::move(event));

            winInfo->m_eventCallback(wrappedEvent);
        });
}

//-------------------------------------------------------------------------------------------------
WindowSystem::~WindowSystem() noexcept
{
	auto& winManager = m_context->m_managerHolder.getManager<WindowManager>();

	if (winManager.m_window != nullptr)
	{
		glfwDestroyWindow(winManager.m_window);
		glfwTerminate();
	}
}

//-------------------------------------------------------------------------------------------------
void WindowSystem::update(float dt)
{
	glfwPollEvents();
}

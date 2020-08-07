/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

//Port of OpenGL backend to DiligentEngine backend by dtcxzyw


#define NOMINMAX
#include <Windows.h>

#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/icons.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <nanogui/texture.h>
#include <nanogui/shader.h>
#include <nanogui/renderpass.h>
#include <iostream>
#include <memory>
#include <stb_image.h>

using namespace nanogui;

class GUI final : public Screen {
public:
    GUI(GLFWwindow *win, NVGcontext *nvg) : Screen() {
        inc_ref();

        Screen::initialize(win, true, nvg);

        Window *window = new Window(this, "Button demo");
        window->set_position(Vector2i(15, 15));
        window->set_layout(new GroupLayout());

        /* No need to store a pointer, the data structure will be automatically
           freed when the parent window is deleted */
        new Label(window, "Push buttons", "sans-bold");

        Button *b = new Button(window, "Plain button");
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("short tooltip");

        /* Alternative construction notation using variadic template */
        b = window->add<Button>("Styled", FA_ROCKET);
        b->set_background_color(Color(0, 0, 255, 25));
        b->set_callback([] { std::cout << "pushed!" << std::endl; });
        b->set_tooltip("This button has a fairly long tooltip. It is so long, in "
            "fact, that the shown text will span several lines.");

        new Label(window, "Toggle buttons", "sans-bold");
        b = new Button(window, "Toggle me");
        b->set_flags(Button::ToggleButton);
        b->set_change_callback([] (bool state) { std::cout << "Toggle button state: " << state << std::endl; });

        new Label(window, "Radio buttons", "sans-bold");
        b = new Button(window, "Radio button 1");
        b->set_flags(Button::RadioButton);
        b = new Button(window, "Radio button 2");
        b->set_flags(Button::RadioButton);

        new Label(window, "A tool palette", "sans-bold");
        Widget *tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Middle, 0, 6));

        b = new ToolButton(tools, FA_CLOUD);
        b = new ToolButton(tools, FA_FAST_FORWARD);
        b = new ToolButton(tools, FA_COMPASS);
        b = new ToolButton(tools, FA_UTENSILS);

        new Label(window, "Popup buttons", "sans-bold");
        PopupButton *popup_btn = new PopupButton(window, "Popup", FA_FLASK);
        Popup *popup = popup_btn->popup();
        popup->set_layout(new GroupLayout());
        new Label(popup, "Arbitrary widgets can be placed here");
        new CheckBox(popup, "A check box");
        // popup right
        popup_btn = new PopupButton(popup, "Recursive popup", FA_CHART_PIE);
        Popup *popup_right = popup_btn->popup();
        popup_right->set_layout(new GroupLayout());
        new CheckBox(popup_right, "Another check box");
        // popup left
        popup_btn = new PopupButton(popup, "Recursive popup", FA_DNA);
        popup_btn->set_side(Popup::Side::Left);
        Popup *popup_left = popup_btn->popup();
        popup_left->set_layout(new GroupLayout());
        new CheckBox(popup_left, "Another check box");

        window = new Window(this, "Basic widgets");
        window->set_position(Vector2i(200, 15));
        window->set_layout(new GroupLayout());

        new Label(window, "Message dialog", "sans-bold");
        tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Middle, 0, 6));
        b = new Button(tools, "Info");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "This is an information message");
            dlg->set_callback([] (int result) { std::cout << "Dialog result: " << result << std::endl; });
            });
        b = new Button(tools, "Warn");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a warning message");
            dlg->set_callback([] (int result) { std::cout << "Dialog result: " << result << std::endl; });
            });
        b = new Button(tools, "Ask");
        b->set_callback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true);
            dlg->set_callback([] (int result) { std::cout << "Dialog result: " << result << std::endl; });
            });

#if defined(_WIN32)
        /// Executable is in the Debug/Release/.. subdirectory
        std::string resources_folder_path("../icons");
#else
        std::string resources_folder_path("./icons");
#endif
        std::vector<std::pair<int, std::string>> icons;

#if !defined(EMSCRIPTEN)
        try {
            icons = load_image_directory(m_nvg_context, resources_folder_path);
        }
        catch (const std::exception &e) {
            std::cerr << "Warning: " << e.what() << std::endl;
        }
#endif

        new Label(window, "Image panel & scroll panel", "sans-bold");
        PopupButton *image_panel_btn = new PopupButton(window, "Image Panel");
        image_panel_btn->set_icon(FA_IMAGES);
        popup = image_panel_btn->popup();
        VScrollPanel *vscroll = new VScrollPanel(popup);
        ImagePanel *img_panel = new ImagePanel(vscroll);
        img_panel->set_images(icons);
        popup->set_fixed_size(Vector2i(245, 150));

        auto image_window = new Window(this, "Selected image");
        image_window->set_position(Vector2i(710, 15));
        image_window->set_layout(new GroupLayout(3));

        /*
        // Create a Texture instance for each object
        for (auto &icon : icons) {
            Vector2i size;
            int n = 0;
            ImageHolder texture_data(
                stbi_load((icon.second + ".png").c_str(), &size.x(), &size.y(), &n, 0),
                stbi_image_free);
            assert(n == 4);

            Texture *tex = new Texture(
                Texture::PixelFormat::RGBA,
                Texture::ComponentFormat::UInt8,
                size,
                Texture::InterpolationMode::Trilinear,
                Texture::InterpolationMode::Nearest);

            tex->upload(texture_data.get());

            m_images.emplace_back(tex, std::move(texture_data));
        }

        ImageView *image_view = new ImageView(image_window);
        if (!m_images.empty())
            image_view->set_image(m_images[0].first);
        image_view->center();
        m_current_image = 0;
        

        img_panel->set_callback([this, image_view] (int i) {
            std::cout << "Selected item " << i << std::endl;
            image_view->set_image(m_images[i].first);
            m_current_image = i;
            });*/

            /*
        image_view->set_pixel_callback(
            [this] (const Vector2i &index, char **out, size_t size) {
                const Texture *texture = m_images[m_current_image].first.get();
                uint8_t *data = m_images[m_current_image].second.get();
                for (int ch = 0; ch < 4; ++ch) {
                    uint8_t value = data[(index.x() + index.y() * texture->size().x()) * 4 + ch];
                    snprintf(out[ch], size, "%i", (int) value);
                }
            }
        );
        */

        new Label(window, "File dialog", "sans-bold");
        tools = new Widget(window);
        tools->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Middle, 0, 6));
        b = new Button(tools, "Open");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                { { "png", "Portable Network Graphics" }, { "txt", "Text file" } }, false) << std::endl;
            });
        b = new Button(tools, "Save");
        b->set_callback([&] {
            std::cout << "File dialog result: " << file_dialog(
                { { "png", "Portable Network Graphics" }, { "txt", "Text file" } }, true) << std::endl;
            });

        new Label(window, "Combo box", "sans-bold");
        new ComboBox(window, { "Combo box item 1", "Combo box item 2", "Combo box item 3" });
        new Label(window, "Check box", "sans-bold");
        CheckBox *cb = new CheckBox(window, "Flag 1",
            [] (bool state) { std::cout << "Check box 1 state: " << state << std::endl; }
        );
        cb->set_checked(true);
        cb = new CheckBox(window, "Flag 2",
            [] (bool state) { std::cout << "Check box 2 state: " << state << std::endl; }
        );
        new Label(window, "Progress bar", "sans-bold");
        m_progress = new ProgressBar(window);

        new Label(window, "Slider and text box", "sans-bold");

        Widget *panel = new Widget(window);
        panel->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Middle, 0, 20));

        Slider *slider = new Slider(panel);
        slider->set_value(0.5f);
        slider->set_fixed_width(80);

        TextBox *text_box = new TextBox(panel);
        text_box->set_fixed_size(Vector2i(60, 25));
        text_box->set_value("50");
        text_box->set_units("%");
        slider->set_callback([text_box] (float value) {
            text_box->set_value(std::to_string((int) (value * 100)));
            });
        slider->set_final_callback([&] (float value) {
            std::cout << "Final slider value: " << (int) (value * 100) << std::endl;
            });
        text_box->set_fixed_size(Vector2i(60, 25));
        text_box->set_font_size(20);
        text_box->set_alignment(TextBox::Alignment::Right);

        window = new Window(this, "Misc. widgets");
        window->set_position(Vector2i(425, 15));
        window->set_layout(new GroupLayout());

        TabWidget *tab_widget = window->add<TabWidget>();

        Widget *layer = new Widget(tab_widget);
        layer->set_layout(new GroupLayout());
        tab_widget->append_tab("Color Wheel", layer);

        // Use overloaded variadic add to fill the tab widget with Different tabs.
        layer->add<Label>("Color wheel widget", "sans-bold");
        layer->add<ColorWheel>();

        layer = new Widget(tab_widget);
        layer->set_layout(new GroupLayout());
        tab_widget->append_tab("Function Graph", layer);

        layer->add<Label>("Function graph widget", "sans-bold");

        Graph *graph = layer->add<Graph>("Some Function");

        graph->set_header("E = 2.35e-3");
        graph->set_footer("Iteration 89");
        std::vector<float> &func = graph->values();
        func.resize(100);
        for (int i = 0; i < 100; ++i)
            func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
            0.5f * std::cos(i / 23.f) + 1);

// Dummy tab used to represent the last tab button.
        int plus_id = tab_widget->append_tab("+", new Widget(tab_widget));

        // A simple counter.
        int counter = 1;
        tab_widget->set_callback([tab_widget, this, counter, plus_id] (int id) mutable {
            if (id == plus_id) {
                // When the "+" tab has been clicked, simply add a new tab.
                std::string tab_name = "Dynamic " + std::to_string(counter);
                Widget *layer_dyn = new Widget(tab_widget);
                int new_id = tab_widget->insert_tab(tab_widget->tab_count() - 1,
                    tab_name, layer_dyn);
                layer_dyn->set_layout(new GroupLayout());
                layer_dyn->add<Label>("Function graph widget", "sans-bold");
                Graph *graph_dyn = layer_dyn->add<Graph>("Dynamic function");

                graph_dyn->set_header("E = 2.35e-3");
                graph_dyn->set_footer("Iteration " + std::to_string(new_id * counter));
                std::vector<float> &func_dyn = graph_dyn->values();
                func_dyn.resize(100);
                for (int i = 0; i < 100; ++i)
                    func_dyn[i] = 0.5f *
                    std::abs((0.5f * std::sin(i / 10.f + counter) +
                    0.5f * std::cos(i / 23.f + 1 + counter)));
                ++counter;
                tab_widget->set_selected_id(new_id);

                // We must invoke the layout manager after adding tabs dynamically
                perform_layout();
            }
            });

            // A button to go back to the first tab and scroll the window.
        panel = window->add<Widget>();
        panel->add<Label>("Jump to tab: ");
        panel->set_layout(new BoxLayout(Orientation::Horizontal,
            Alignment::Middle, 0, 6));

        auto ib = panel->add<IntBox<int>>();
        ib->set_editable(true);

        b = panel->add<Button>("", FA_FORWARD);
        b->set_fixed_size(Vector2i(22, 22));
        ib->set_fixed_height(22);
        b->set_callback([tab_widget, ib] {
            int value = ib->value();
            if (value >= 0 && value < tab_widget->tab_count())
                tab_widget->set_selected_index(value);
            });

        window = new Window(this, "Grid of small widgets");
        window->set_position(Vector2i(425, 300));
        GridLayout *layout =
            new GridLayout(Orientation::Horizontal, 2,
            Alignment::Middle, 15, 5);
        layout->set_col_alignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->set_spacing(0, 10);
        window->set_layout(layout);

        /* FP widget */
        {
            new Label(window, "Floating point :", "sans-bold");
            text_box = new TextBox(window);
            text_box->set_editable(true);
            text_box->set_fixed_size(Vector2i(100, 20));
            text_box->set_value("50");
            text_box->set_units("GiB");
            text_box->set_default_value("0.0");
            text_box->set_font_size(16);
            text_box->set_format("[-]?[0-9]*\\.?[0-9]+");
        }

        /* Positive integer widget */
        {
            new Label(window, "Positive integer :", "sans-bold");
            auto int_box = new IntBox<int>(window);
            int_box->set_editable(true);
            int_box->set_fixed_size(Vector2i(100, 20));
            int_box->set_value(50);
            int_box->set_units("Mhz");
            int_box->set_default_value("0");
            int_box->set_font_size(16);
            int_box->set_format("[1-9][0-9]*");
            int_box->set_spinnable(true);
            int_box->set_min_value(1);
            int_box->set_value_increment(2);
        }

        /* Checkbox widget */
        {
            new Label(window, "Checkbox :", "sans-bold");

            cb = new CheckBox(window, "Check me");
            cb->set_font_size(16);
            cb->set_checked(true);
        }

        new Label(window, "Combo box :", "sans-bold");
        ComboBox *cobo =
            new ComboBox(window, { "Item 1", "Item 2", "Item 3" });
        cobo->set_font_size(16);
        cobo->set_fixed_size(Vector2i(100, 20));

        new Label(window, "Color picker :", "sans-bold");
        auto cp = new ColorPicker(window, { 255, 120, 0, 255 });
        cp->set_fixed_size({ 100, 20 });
        cp->set_final_callback([] (const Color &c) {
            std::cout << "ColorPicker final callback: ["
                << c.r() << ", "
                << c.g() << ", "
                << c.b() << ", "
                << c.w() << "]" << std::endl;
            });
            // setup a fast callback for the color picker widget on a new window
            // for demonstrative purposes
        window = new Window(this, "Color Picker Fast Callback");
        layout = new GridLayout(Orientation::Horizontal, 2,
            Alignment::Middle, 15, 5);
        layout->set_col_alignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->set_spacing(0, 10);
        window->set_layout(layout);
        window->set_position(Vector2i(425, 500));
        new Label(window, "Combined: ");
        b = new Button(window, "ColorWheel", FA_INFINITY);
        new Label(window, "Red: ");
        auto red_int_box = new IntBox<int>(window);
        red_int_box->set_editable(false);
        new Label(window, "Green: ");
        auto green_int_box = new IntBox<int>(window);
        green_int_box->set_editable(false);
        new Label(window, "Blue: ");
        auto blue_int_box = new IntBox<int>(window);
        blue_int_box->set_editable(false);
        new Label(window, "Alpha: ");
        auto alpha_int_box = new IntBox<int>(window);

        cp->set_callback([b, red_int_box, blue_int_box, green_int_box, alpha_int_box] (const Color &c) {
            b->set_background_color(c);
            b->set_text_color(c.contrasting_color());
            int red = (int) (c.r() * 255.0f);
            red_int_box->set_value(red);
            int green = (int) (c.g() * 255.0f);
            green_int_box->set_value(green);
            int blue = (int) (c.b() * 255.0f);
            blue_int_box->set_value(blue);
            int alpha = (int) (c.w() * 255.0f);
            alpha_int_box->set_value(alpha);
            });

        perform_layout();

        /* All NanoGUI widgets are initialized at this point. Now
           create shaders to draw the main window contents.

           NanoGUI comes with a simple wrapper around OpenGL 3, which
           eliminates most of the tedious and error-prone shader and buffer
           object management.
        */
    }

    virtual bool keyboard_event(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboard_event(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            set_visible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Animate the scrollbar */
        m_progress->set_value(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

private:
    ProgressBar *m_progress;
};

#include <nanovg_DE.hpp>

#define ENGINE_DLL 1
#define D3D11_SUPPORTED 1
#define D3D12_SUPPORTED 1
#define GL_SUPPORTED 1
#define VULKAN_SUPPORTED 1

#include <DiligentCore/Common/interface/FileWrapper.hpp>
#include <DiligentCore/Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#include <DiligentCore/Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#include <DiligentCore/Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>
#include <DiligentCore/Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#include <DiligentCore/Graphics/GraphicsTools/interface/DurationQueryHelper.hpp>
#include <DiligentCore/Graphics/GraphicsTools/interface/ScreenCapture.hpp>
#include <DiligentTools/TextureLoader/interface/Image.h>
#include <chrono>
#include <sstream>

using Clock = std::chrono::high_resolution_clock;

#ifdef _DEBUG
#define DILIGENT_DEBUG
#endif  // _DEBUG

#define NANOGUI_MSAA

static void callback(DE::DEBUG_MESSAGE_SEVERITY severity, const char *message,
    const char *function, const char *file, int line) {
    if (severity != DE::DEBUG_MESSAGE_SEVERITY_INFO) {
        DebugBreak();
    }
    else
        OutputDebugStringA(message);
}

class Engine final {
public:
    Engine(void *hWnd, DE::RENDER_DEVICE_TYPE type) {
        DE::SwapChainDesc SCDesc;
        SCDesc.DefaultDepthValue = 1.0f;
        SCDesc.DefaultStencilValue = 0;
        SCDesc.ColorBufferFormat = DE::TEX_FORMAT_RGBA8_UNORM;
        SCDesc.DepthBufferFormat = DE::TEX_FORMAT_D24_UNORM_S8_UINT;
        SCDesc.Usage = DE::SWAP_CHAIN_USAGE_RENDER_TARGET |
            DE::SWAP_CHAIN_USAGE_COPY_SOURCE;

        switch (type) {
#if D3D11_SUPPORTED
            case DE::RENDER_DEVICE_TYPE_D3D11:
            {
                DE::EngineD3D11CreateInfo EngineCI;
                EngineCI.DebugMessageCallback = callback;
#ifdef DILIGENT_DEBUG
                EngineCI.DebugFlags |=
                    DE::D3D11_DEBUG_FLAG_CREATE_DEBUG_DEVICE |
                    DE::D3D11_DEBUG_FLAG_VERIFY_COMMITTED_SHADER_RESOURCES;
#endif
#if ENGINE_DLL
                // Load the dll and import GetEngineFactoryD3D11() function
                auto GetEngineFactoryD3D11 = DE::LoadGraphicsEngineD3D11();
#endif
                auto *pFactoryD3D11 = GetEngineFactoryD3D11();
                pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &device,
                    &immediateContext);
                DE::Win32NativeWindow window{ hWnd };
                pFactoryD3D11->CreateSwapChainD3D11(
                    device, immediateContext, SCDesc, DE::FullScreenModeDesc{},
                    window, &swapChain);
            } break;
#endif

#if D3D12_SUPPORTED
            case DE::RENDER_DEVICE_TYPE_D3D12:
            {
#if ENGINE_DLL
                // Load the dll and import GetEngineFactoryD3D12() function
                auto GetEngineFactoryD3D12 = DE::LoadGraphicsEngineD3D12();
#endif
                DE::EngineD3D12CreateInfo EngineCI;
                EngineCI.DebugMessageCallback = callback;
#ifdef DILIGENT_DEBUG
                EngineCI.EnableDebugLayer = true;
#endif
                auto *pFactoryD3D12 = GetEngineFactoryD3D12();
                pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &device,
                    &immediateContext);
                DE::Win32NativeWindow window{ hWnd };
                pFactoryD3D12->CreateSwapChainD3D12(
                    device, immediateContext, SCDesc, DE::FullScreenModeDesc{},
                    window, &swapChain);
            } break;
#endif

#if GL_SUPPORTED
            case DE::RENDER_DEVICE_TYPE_GL:
            {

#if EXPLICITLY_LOAD_ENGINE_GL_DLL
                // Load the dll and import GetEngineFactoryOpenGL() function
                auto GetEngineFactoryOpenGL = DE::LoadGraphicsEngineOpenGL();
#endif
                auto *pFactoryOpenGL = GetEngineFactoryOpenGL();

                DE::EngineGLCreateInfo EngineCI;
                EngineCI.Window.hWnd = hWnd;
                EngineCI.DebugMessageCallback = callback;
#ifdef DILIGENT_DEBUG
                EngineCI.CreateDebugContext = true;
#endif
                pFactoryOpenGL->CreateDeviceAndSwapChainGL(
                    EngineCI, &device, &immediateContext, SCDesc, &swapChain);
            } break;
#endif

#if VULKAN_SUPPORTED
            case DE::RENDER_DEVICE_TYPE_VULKAN:
            {
#if EXPLICITLY_LOAD_ENGINE_VK_DLL
                // Load the dll and import GetEngineFactoryVk() function
                auto GetEngineFactoryVk = DE::LoadGraphicsEngineVk();
#endif
                DE::EngineVkCreateInfo EngineCI;
                EngineCI.DebugMessageCallback = callback;
#ifdef DILIGENT_DEBUG
                EngineCI.EnableValidation = true;
#endif
                auto *pFactoryVk = GetEngineFactoryVk();
                pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &device,
                    &immediateContext);

                if (!swapChain && hWnd != nullptr) {
                    DE::Win32NativeWindow window{ hWnd };
                    pFactoryVk->CreateSwapChainVk(device, immediateContext,
                        SCDesc, window, &swapChain);
                }
            } break;
#endif

            default:
                throw std::logic_error("Unknown/unsupported device type");
                break;
        }
    }

    ~Engine() {
        immediateContext->Flush();
    }

    void updateTarget(const float *clearColor) {
        auto *pRTV = swapChain->GetCurrentBackBufferRTV();
        auto *pDSV = swapChain->GetDepthBufferDSV();
        if (sampleCount) {
            pRTV = msaaColorRTV;
            pDSV = msaaDepthDSV;
        }

        immediateContext->SetRenderTargets(
            1, &pRTV, pDSV, DE::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Clear the back buffer
        // Let the engine perform required state transitions
        immediateContext->ClearRenderTarget(
            pRTV, clearColor, DE::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        immediateContext->ClearDepthStencil(
            pDSV, DE::CLEAR_DEPTH_FLAG | DE::CLEAR_STENCIL_FLAG, 1.0f, 0,
            DE::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }

    void resetRenderTarget() {
        if (sampleCount == 1)
            return;

        const auto &SCDesc = swapChain->GetDesc();
        // Create window-size multi-sampled offscreen render target
        DE::TextureDesc colTexDesc = {};
        colTexDesc.Name = "Color RTV";
        colTexDesc.Type = DE::RESOURCE_DIM_TEX_2D;
        colTexDesc.BindFlags = DE::BIND_RENDER_TARGET;
        colTexDesc.Width = SCDesc.Width;
        colTexDesc.Height = SCDesc.Height;
        colTexDesc.MipLevels = 1;
        colTexDesc.Format = SCDesc.ColorBufferFormat;
        bool needSRGBConversion = device->GetDeviceCaps().IsD3DDevice() &&
            (colTexDesc.Format == DE::TEX_FORMAT_RGBA8_UNORM_SRGB ||
            colTexDesc.Format == DE::TEX_FORMAT_BGRA8_UNORM_SRGB);
        if (needSRGBConversion) {
            // Internally Direct3D swap chain images are not SRGB, and
            // ResolveSubresource requires source and destination formats to
            // match exactly or be typeless. So we will have to create a
            // typeless texture and use SRGB render target view with it.
            colTexDesc.Format =
                colTexDesc.Format == DE::TEX_FORMAT_RGBA8_UNORM_SRGB ?
                DE::TEX_FORMAT_RGBA8_TYPELESS :
                DE::TEX_FORMAT_BGRA8_TYPELESS;
        }

        // Set the desired number of samples
        colTexDesc.SampleCount = sampleCount;
        // Define optimal clear value
        float col[4] = { 0.3f, 0.3f, 0.32f, 1.0f };
        memcpy(colTexDesc.ClearValue.Color, col, sizeof(col));
        colTexDesc.ClearValue.Format = SCDesc.ColorBufferFormat;
        DE::RefCntAutoPtr<DE::ITexture> pColor;
        device->CreateTexture(colTexDesc, nullptr, &pColor);

        // Store the render target view
        msaaColorRTV.Release();
        if (needSRGBConversion) {
            DE::TextureViewDesc RTVDesc;
            RTVDesc.ViewType = DE::TEXTURE_VIEW_RENDER_TARGET;
            RTVDesc.Format = SCDesc.ColorBufferFormat;
            pColor->CreateView(RTVDesc, &msaaColorRTV);
        }
        else {
            msaaColorRTV =
                pColor->GetDefaultView(DE::TEXTURE_VIEW_RENDER_TARGET);
        }

        // Create window-size multi-sampled depth buffer
        DE::TextureDesc depthDesc = colTexDesc;
        depthDesc.Name = "depth DSV";
        depthDesc.Format = SCDesc.DepthBufferFormat;
        depthDesc.BindFlags = DE::BIND_DEPTH_STENCIL;
        // Define optimal clear value
        depthDesc.ClearValue.Format = depthDesc.Format;

        DE::RefCntAutoPtr<DE::ITexture> pDepth;
        device->CreateTexture(depthDesc, nullptr, &pDepth);
        // Store the depth-stencil view
        msaaDepthDSV = pDepth->GetDefaultView(DE::TEXTURE_VIEW_DEPTH_STENCIL);
    }

    DE::RefCntAutoPtr<DE::IRenderDevice> device;
    DE::RefCntAutoPtr<DE::IDeviceContext> immediateContext;
    DE::RefCntAutoPtr<DE::ISwapChain> swapChain;
    DE::Uint32 sampleCount = 1;
    DE::RefCntAutoPtr<DE::ITextureView> msaaColorRTV;
    DE::RefCntAutoPtr<DE::ITextureView> msaaDepthDSV;
};

std::unique_ptr<Engine> gEngine;

#ifdef D3D11_SUPPORTED
#include <d3d11.h>
#endif  // D3D11_SUPPORTED
#ifdef D3D12_SUPPORTED
#include <d3d12.h>
#endif  // D3D12_SUPPORTED

DE::Uint8 getQualityLevel() {
    auto dev = gEngine->device->GetDeviceCaps().DevType;
    void *nativeHandle = gEngine->swapChain->GetCurrentBackBufferRTV()
        ->GetTexture()
        ->GetNativeHandle();
#ifdef D3D11_SUPPORTED
    if (dev == DE::RENDER_DEVICE_TYPE_D3D11) {
        auto res = reinterpret_cast<ID3D11Resource *>(nativeHandle);
        ID3D11Device *device = nullptr;
        res->GetDevice(&device);
        UINT level = 0;
        device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM,
            gEngine->sampleCount, &level);
        return static_cast<DE::Uint8>(level - 1);
    }
#endif  // D3D11_SUPPORTED
#ifdef D3D12_SUPPORTED
    if (dev == DE::RENDER_DEVICE_TYPE_D3D12) {
        auto res = reinterpret_cast<ID3D12Resource *>(nativeHandle);
        void *device = nullptr;
        res->GetDevice(__uuidof(ID3D12Device), &device);
        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS
            data = {};
        data.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        data.SampleCount = gEngine->sampleCount;
        data.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        reinterpret_cast<ID3D12Device *>(device)->CheckFeatureSupport(
            D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &data, sizeof(data));
        return static_cast<DE::Uint8>(data.NumQualityLevels - 1);
    }
#endif  // D3D12_SUPPORTED
    return 0;
}

#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>

ref<GUI> gGUI;

void glfwCursorPosCallback(GLFWwindow *w, double x, double y) {
    gGUI->cursor_pos_callback_event(x, y);
}

void glfwMouseButtonCallback(GLFWwindow *w, int button, int action, int modifiers) {
    gGUI->mouse_button_callback_event(button, action, modifiers);
}

void glfwKeyCallback(GLFWwindow *w, int key, int scancode, int action, int mods) {
        gGUI->key_callback_event(key, scancode, action, mods);
}

void glfwCharCallback(GLFWwindow *w, unsigned int codepoint) {
        gGUI->char_callback_event(codepoint);
}

void glfwDropCallback(GLFWwindow *w, int count, const char **filenames) {
        gGUI->drop_callback_event(count, filenames);
}

void glfwScrollCallback(GLFWwindow *w, double x, double y) {
        gGUI->scroll_callback_event(x, y);
}

/* React to framebuffer size events -- includes window
   size events and also catches things like dragging
   a window from a Retina-capable screen to a normal
   screen on Mac OS X */
void glfwFramebufferSizeCallback (GLFWwindow *w, int width, int height) {
        gEngine->swapChain->Resize(width, height);
        gGUI->resize_callback_event(width, height);
}

// notify when the screen has lost focus (e.g. gGUIlication switch)
void glfwWindowFocusCallback(GLFWwindow *w, int focused) {
        // focus_event: 0 when false, 1 when true
        gGUI->focus_event(focused != 0);
}

int main(int /* argc */, char ** /* argv */) {
    try {
        glfwSetErrorCallback(
            [] (int error, const char *descr) {
                if (error == GLFW_NOT_INITIALIZED)
                    return; /* Ignore */
                std::cerr << "GLFW error " << error << ": " << descr << std::endl;
            }
        );

        if (!glfwInit())
            throw std::runtime_error("Could not initialize GLFW!");

        glfwSetTime(0);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        auto window = glfwCreateWindow(1024, 768, "NanoGUI Test", nullptr, nullptr);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        gEngine = std::make_unique<Engine>(glfwGetWin32Window(window), DE::RENDER_DEVICE_TYPE_VULKAN);

        auto &&SDesc = gEngine->swapChain->GetDesc();

#ifdef NANOGUI_MSAA
        const auto &colorFmtInfo =
            gEngine->device->GetTextureFormatInfoExt(SDesc.ColorBufferFormat);
        const auto &depthFmtInfo =
            gEngine->device->GetTextureFormatInfoExt(SDesc.DepthBufferFormat);
        DE::Uint32 supportedSampleCounts =
            colorFmtInfo.SampleCounts & depthFmtInfo.SampleCounts;
        while (supportedSampleCounts & (gEngine->sampleCount << 1))
            gEngine->sampleCount <<= 1;
#endif  // NANOGUI_MSAA

        if (gEngine->sampleCount > 1)
            gEngine->resetRenderTarget();

        DE::SampleDesc msaa = {};
        msaa.Count = gEngine->sampleCount;
        msaa.Quality = getQualityLevel();

        NVGcontext *nvg = nvgCreateDE(
            gEngine->device, gEngine->immediateContext, msaa,
            SDesc.ColorBufferFormat, SDesc.DepthBufferFormat,
            static_cast<int>((msaa.Count == 1 ? NVGCreateFlags::NVG_ANTIALIAS : 0)
            | NVG_ALLOW_INDIRECT_RENDERING
            | NVGCreateFlags::NVG_STENCIL_STROKES
#ifdef _DEBUG
            | NVGCreateFlags::NVG_DEBUG
#endif
        ));

        /* scoped variables */
        {
            gGUI = new GUI(window, nvg);

            glfwSetCursorPosCallback(window, glfwCursorPosCallback);
            glfwSetMouseButtonCallback(window,glfwMouseButtonCallback);
            glfwSetKeyCallback(window,glfwKeyCallback);
            glfwSetCharCallback(window,glfwCharCallback);
            glfwSetDropCallback(window,glfwDropCallback);
            glfwSetScrollCallback(window,glfwScrollCallback);

            /* React to framebuffer size events -- includes window
               size events and also catches things like dragging
               a window from a Retina-capable screen to a normal
               screen on Mac OS X */
            glfwSetFramebufferSizeCallback(window,glfwFramebufferSizeCallback);

            // notify when the screen has lost focus (e.g. application switch)
            glfwSetWindowFocusCallback(window,glfwWindowFocusCallback);


            gGUI->dec_ref();
            gGUI->set_visible(true);

            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                const float clearCol[] = { 0.3f, 0.3f, 0.32f, 1.0f };
                gEngine->updateTarget(clearCol);

                gGUI->draw_setup();
                gGUI->draw_widgets();

                if (gEngine->sampleCount > 1) {
                    // Resolve multi-sampled render taget into the current swap
                    // chain back buffer.
                    auto pCurrentBackBuffer =
                        gEngine->swapChain->GetCurrentBackBufferRTV()->GetTexture();

                    DE::ResolveTextureSubresourceAttribs RA = {};
                    RA.SrcTextureTransitionMode =
                        DE::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                    RA.DstTextureTransitionMode =
                        DE::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
                    gEngine->immediateContext->ResolveTextureSubresource(
                        gEngine->msaaColorRTV->GetTexture(), pCurrentBackBuffer,
                        RA);
                }

                gEngine->swapChain->Present(0U);
            }
        }

        nvgDeleteDE(nvg);
        gEngine.reset();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
#if defined(_WIN32)
        MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
#else
        std::cerr << error_msg << std::endl;
#endif
        return -1;
    }
    catch (...) {
        std::cerr << "Caught an unknown error!" << std::endl;
    }

    return 0;
}

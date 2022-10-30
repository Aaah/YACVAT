#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#include <filesystem>
#include "getopt.h"
#include "version.h"
#include "app.h"             // context application
#include "ImGuiFileDialog.h" // add-on filedialogs
#include "spdlog/spdlog.h"   // logs

// context application
AnnotationApp app;

int ImGui_AnnotationTool(void)
{
    // todo : resize panels when resizing the window

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin("Annotation Tool", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

    ImGui::BeginChild("Pane2", ImVec2(200, -1.f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Images Menu"))
        {
            if (ImGui::MenuItem("Open folder"))
                app.open_images_folder_flag = true;

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (app.open_images_folder_flag == true)
    {
        ImGuiFileDialog::Instance()->OpenDialog("FolderChooser", "Choose a Directory", nullptr, ".", ImGuiFileDialogFlags_Modal);

        // display
        if (ImGuiFileDialog::Instance()->Display("FolderChooser"))
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                app.update_images_folder(ImGuiFileDialog::Instance()->GetCurrentPath());
            }

            // close
            ImGuiFileDialog::Instance()->Close();
            app.open_images_folder_flag = false;
        }
    }

    // display images files
    app.ui_images_folder();

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Pane3", ImVec2(300, -1.f), true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Annotations"))
        {
            // if (ImGui::MenuItem("Open/create file"))
            // {
            // }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    app.ui_annotations_panel();

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Pane1", ImVec2(-1.f, -1.f), false, ImGuiWindowFlags_AlwaysAutoResize);

    app.ui_image_current();

    ImGui::EndChild();


    ImGui::PopStyleVar();
    ImGui::End();
    
    return 0;
}

// Main code
int main(int argc, char **argv)
{
    // logs level
    spdlog::set_level(spdlog::level::debug);

    int c;
    int verbose_flag = 0;
    int version_flag = 0;
    static struct option long_options[] =
        {
            {"verbose", no_argument, &verbose_flag, 1},
            {"version", no_argument, &version_flag, 1}};

    int option_index;
    while ((c = getopt_long(argc, argv, ":::", long_options, &option_index)) != -1)
    {
        switch (c)
        {
        case 0:
        {
            if (version_flag)
            {
                spdlog::info("Version of YACVAT is {}.{}.{}", YACVAT_VER_MAJOR, YACVAT_VER_MINOR, YACVAT_VER_PATCH);
            }

            spdlog::set_level(spdlog::level::critical);
            if (verbose_flag)
            {
                spdlog::set_level(spdlog::level::debug);
            }

            break;
        }
        default:
        {
            break;
        }
        }
    }

    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char *glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    char wname[64] = "\0";
    sprintf(wname, "YACVAT version %d.%d.%d", YACVAT_VER_MAJOR, YACVAT_VER_MINOR, YACVAT_VER_PATCH);
    SDL_Window *window = SDL_CreateWindow(wname, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("assets/NotoSans-Regular.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool show_metrics = false;
    // bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // to enable dragging on the image without moving the window around
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        if (show_metrics)
            ImGui::ShowMetricsWindow(&show_metrics);

        ImGui_AnnotationTool();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
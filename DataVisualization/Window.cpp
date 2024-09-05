#include "Window.h"

#define NOMINMAX

std::vector<Drawable*> Window::managedList{};
Drawable* Window::s = nullptr;
Drawable* Window::focus = nullptr;
Form Window::form{};

bool Window::drawColorPicker = false;
void cursorCallback(GLFWwindow* window, int button, int action, int mods);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void windowResizeCallback (GLFWwindow* window, int width, int height);
void frameBufferCallback(GLFWwindow* window, int width, int height);

Window::Window () {
    std::cout << "Creating window..." << std::endl;
    // init glfw
    initGLFW();

    // create window
    window = glfwCreateWindow((int)config::windowX, (int)config::windowY, "Data Visualization", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glViewport(0, 0, config::windowX, config::windowY);
    //

    // init glew
    initGLEW();

    glfwSetMouseButtonCallback(window, cursorCallback);
    glfwSetKeyCallback(window, keyCallBack);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetWindowSizeCallback(window, windowResizeCallback);
    glfwSetFramebufferSizeCallback(window, frameBufferCallback);
    
    initImGui();

    for (int a = 0; a < config::maxClassValue; a++) {
        float val = 0.8f * (((float)a+1)/(config::maxClassValue+1));
        config::classColors.insert(config::classColors.end(), ImVec4{val, val, val, 1.0f});
    }
}


// ensure that GLEW was successful
void Window::initGLEW () {
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cout << glewGetErrorString(err) << std::endl;
    }
    else {
        std::cout << "GLEW OK!" << std::endl;
    }
}

// ensure that GLFW was successful
void Window::initGLFW () {
    if (!glfwInit()) {
        exit(1);
    }
    else {
        std::cout << "GLFW OK!" << std::endl;
    }
}

void Window::initImGui () {
    // create the ImGui context and set some settings
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    flags |= ImGuiWindowFlags_NoMove;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    font = io.Fonts->AddFontFromFileTTF("resources/fonts/ProggyClean.ttf", 13.0f);
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
}

void Window::endImGui () {
    // end the ImGui context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
}

void Window::drawImGuiWindow(Texture& texture) {
    // create a new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::PushFont(font);
    createOptions(texture);
    if (!Window::form.open) {
        createColorPicker();
    }
    else {
        form.draw();
    }
    ImGui::PopFont();

    // render and update
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
}

// function that is called when the mouse is clicked
void cursorCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
        Window::focus = Window::s;
        Window::drawColorPicker = true;
    }
}

// function that is called when a key is pressed
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // cleanup later but fine in short term.
    if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::managedList[config::drawIndex]->setTranslation(Window::managedList[config::drawIndex]->getX(), Window::managedList[config::drawIndex]->getY() - 5);
    }
    if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::managedList[config::drawIndex]->setTranslation(Window::managedList[config::drawIndex]->getX(), Window::managedList[config::drawIndex]->getY() + 5);
    }
    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::managedList[config::drawIndex]->setTranslation(Window::managedList[config::drawIndex]->getX() - 5, Window::managedList[config::drawIndex]->getY());
    }
    if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::managedList[config::drawIndex]->setTranslation(Window::managedList[config::drawIndex]->getX() + 5, Window::managedList[config::drawIndex]->getY());
    }
    if (key == GLFW_KEY_KP_ADD && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::managedList[config::drawIndex]->setScale(Window::managedList[config::drawIndex]->getScale() + .02f);
    }
    if (key == GLFW_KEY_KP_SUBTRACT && (action == GLFW_REPEAT || action == GLFW_PRESS || glfwGetKey(window, key))) {
        if ((Window::managedList[config::drawIndex]->getScale() - .02) > 0.00001) {
            Window::managedList[config::drawIndex]->setScale(Window::managedList[config::drawIndex]->getScale() + .02f);
        }
    }
    if (key == GLFW_KEY_ESCAPE && (action == GLFW_PRESS || glfwGetKey(window, key))) {
        Window::form.open = !Window::form.open;
        // prob better in the actual form class, but sets it so if the user presses escape while the form is open on the intro screen, it goes to the prep screen rather than
        // the intro screen again when the window is opened.
        if (Window::form.current == state::INTRODUCTION) {
            Window::form.current = state::FUNCTION;
        }
    }
}

// function that is called when the cursor moves
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Window::s = nullptr;
    for (auto a : Window::managedList) {
        Window::s = a->selected(window);
        if (Window::s != nullptr) {
            break;
        }
    }
}

void windowResizeCallback (GLFWwindow* window, int width, int height) {
    float scale = std::min(config::windowX/width, config::windowY/height);
    config::windowX = width;
    config::windowY = height;
}

void frameBufferCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// creates the table of buttons by sampling a texture
void Window::createOptions (Texture& texture) {
    ImGui::Begin("Options", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    // create options table
    texture.bind();
    int numOfElements = std::min((int)round(config::windowX / 32.0f), (int)config::options.size());
    ImVec2 window = ImGui::GetWindowSize();

    int x, y;
    for (int item = 0; item < config::options.size(); item++) {
        x = item % 5;
        y = item / 5;
        ImGui::SameLine();

        ImVec2 size = ImVec2(config::buttonSize, config::buttonSize);   // Size of the image we want to make visible

        // UV coordinates for lower-left
        ImVec2 uv0 = ImVec2(
            (config::buttonSize * x / texture.getWidth()),
            (config::buttonSize * y / texture.getHeight())
        );

        // UV coordinates for (32,32) in our texture
        ImVec2 uv1 = ImVec2(
            (config::buttonSize * (x + 1) / texture.getWidth()),
            (config::buttonSize * (y + 1) / texture.getHeight())
        );

        ImGui::PushID(item);
        if (ImGui::ImageButton("", (void*)texture.id, size, uv0, uv1)) {
            Window::buttonActions(item);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(config::options.at(item).c_str());
            ImGui::EndTooltip();
        }
        ImGui::PopID();

    }


    ImGui::SetWindowSize(ImVec2(config::windowX, config::buttonSize*2));
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::End();
    
    // bottom row of buttons
    ImGui::Begin("##formState", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::SetWindowSize(ImVec2(config::windowX * .5f, config::windowY * .05f));
    ImGui::SetWindowPos(ImVec2(0.0f, config::windowY * .95f));
    window = ImGui::GetWindowSize();
    ImVec2 buttonSize{window.x * .225f, window.y * .6f};
    if (ImGui::Button("Open Help##", buttonSize)) {
        form.current = state::INTRODUCTION;
        form.openWindow();
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Prep##", buttonSize)) {
        form.current = state::PREP;
        form.openWindow();
    }
    ImGui::SameLine();
    if (ImGui::Button("Edit colors##", buttonSize)) {
        form.current = state::COLOR;
        form.openWindow();
    }
    ImGui::SameLine();
    if (ImGui::Button("Compare Models", buttonSize)) {
        form.current = state::COMPARE;
        form.openWindow();
    }
    ImGui::End();

    ImGui::Begin("##modelswitch", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImVec2 modelSelectWindow = {config::windowX * .2f, config::windowY * .06f};
    ImGui::SetWindowSize(modelSelectWindow);
    ImGui::SetWindowPos(ImVec2(config::windowX - modelSelectWindow.x, config::windowY - modelSelectWindow.y));
    ImGui::Text((std::string("Selected Model: ") + std::to_string(config::drawIndex+1)).c_str());
    ImGui::SetNextItemWidth(modelSelectWindow.x * .5f);
    if (ImGui::ArrowButton("##leftb", ImGuiDir_Left)) {
        if (config::drawIndex > 0) {
            config::drawIndex--;
        }
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##rightb", ImGuiDir_Right)) {
        if (config::drawIndex < managedList.size()-1) {
            config::drawIndex++;
        }
    }

    ImGui::End();
}

// when you click, it checks if we are focusing on a shape, if we are, create the color picker window.
void Window::createColorPicker () {
    if (Window::drawColorPicker && Window::focus != nullptr) {
        // create color picker
        ImGui::Begin("Color Picker", &Window::drawColorPicker);

        if (Window::focus != nullptr) {
            ImGui::ColorPicker4("Shape Color", (float*)Window::focus->getColor(), NULL);
        }
        ImGui::End();
    }
}

// draws the members of the managedList
void Window::draw () {
    if (!managedList.empty()) {
        managedList[config::drawIndex]->draw();
    }
    //for (auto a : managedList) {
    //    a->draw();
    //}
}

void Window::addModelFromForm() {
    const auto classIndex = this->form.getFunc()->hanselChains->dimension - 1;
    Model* m = new Model();

    for (const auto& chain : this->form.getFunc()->hanselChains->hanselChainSet)
    {
        std::vector<int> classes;

        for (const auto& e : chain)
        {
            classes.push_back(e[classIndex]);
        }

        m->addColumn(&classes);
    }

    m->fitToScreen();
    this->addToRender(m);
    this->form.addModel = !this->form.addModel;
}


// when a button is pressed, it calls this function with a "val" equal to which button in the window is pressed.
void Window::buttonActions(int val) {
    switch (val) {
    case (0): {
        std::cout << "beep" << std::endl;
        break;
    }
    case (1): {
        std::cout << "boop" << std::endl;
        break;
    }
    case (2): {
        std::cout << "bop" << std::endl;
        break;
    }
    case (3): {
        break;
    }
    case (4): {
        break;
    }
    case (5): {
        break;
    }
    }
}
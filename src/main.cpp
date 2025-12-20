#include "ToonApp.h"

int main() {
    ToonApp app(1280, 1080, "Toon Shaded Engine");
    // Run
    try {
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

return 0;
}
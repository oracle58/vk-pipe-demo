#include <vkp/graphics/engine.h>
#include <vkp/logger.h>

int main() {
    vkp::graphics::Engine engine;
    vkp::graphics::engine_conf conf;

    conf.start_pos_x = 100;
    conf.start_pos_y = 100;
    conf.start_width = 1280;
    conf.start_height = 720;
    conf.name = "vkpipe demo v1";

    if (!engine.init(conf)) {
        LOG_ERROR("Application failed to create.");
        return 1;
    }
    return engine.run() ? 0 : -1;
}
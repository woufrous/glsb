#pragma once

class Application {
    public:
        Application() : is_running_{true} {}

        void run() {
            while (is_running_) {
                update();
                draw();
            }
        }

        virtual void init() = 0;
        virtual void update() = 0;
        virtual void draw() = 0;
    protected:
        bool is_running_;
};

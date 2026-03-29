#include "Assistant.h"
#include <iostream>
#include <string>
#include <atomic>

std::atomic<bool> running{ true };

int main() {
    std::cout << "[AI Assistant] Запуск на Linux (UTF-8)\n" << std::flush;

    Assistant assistant("mongodb://localhost:27017",
        "http://127.0.0.1:11434",
        Assistant::DEEP_MODEL);

    std::string line;
    std::string current_device = "";

    std::cout << "[AI Assistant] Готов. Ожидаю команды от FastAPI...\n" << std::flush;

    while (running && std::getline(std::cin, line)) {
        if (line.empty()) continue;

        if (line.rfind("DEVICE:", 0) == 0) {
            current_device = line.substr(7);
            auto opt = assistant.findInstrument(current_device);

            if (opt && opt->is_valid()) {
                std::cout << "DEVICE_CONNECTED:" << opt->normalized_name << "\n";
                std::cout << "MESSAGE:Устройство " << opt->normalized_name
                    << " успешно подключено. Задавайте вопросы AI-ассистенту.\n" << std::flush;
            }
            else {
                std::cout << "DEVICE_NOT_FOUND:" << current_device << "\n";
                std::cout << "MESSAGE:Прибор не найден в базе данных.\n" << std::flush;
            }
        }
        else if (line.rfind("QUERY:", 0) == 0 && !current_device.empty()) {
            std::string query = line.substr(6);
            std::cout << "PROCESSING:" << query << "\n" << std::flush;

            std::string answer = assistant.getResponse(query, current_device);
            std::cout << "ANSWER:" << answer << "\n" << std::flush;
        }
        else if (line == "EXIT" || line == "QUIT") {
            break;
        }
    }

    std::cout << "[AI Assistant] Завершение.\n" << std::flush;
    return 0;
}
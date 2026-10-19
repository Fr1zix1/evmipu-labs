#include <iostream>
#include <iomanip>
#include <string>
#include <libusb-1.0/libusb.h> 

using namespace std;

string getClassName(int classCode) {
    switch (classCode) {
        case 0x00: return "код отсутствует";
        case 0x01: return "аудиоустройство";
        case 0x02: return "коммуникационное устройство (сетевой адаптер)";
        case 0x03: return "устройство пользовательского интерфейса";
        case 0x05: return "физическое устройство";
        case 0x06: return "изображения";
        case 0x07: return "принтер";
        case 0x08: return "устройство хранения данных";
        case 0x09: return "концентратор";
        case 0x0A: return "CDC-Data";
        case 0x0B: return "Smart Card";
        case 0x0D: return "Content Security";
        case 0x0E: return "видеоустройство";
        case 0x0F: return "персональное медицинское устройство";
        case 0x10: return "аудио- и видеоустройства";
        case 0x11: return "Billboard-контроллер (Type-C)";
        case 0xDC: return "диагностическое устройство";
        case 0xE0: return "беспроводный контроллер";
        case 0xEF: return "различные устройства";
        case 0xFE: return "специфическое устройство";
        case 0xFF: return "устройство, специфичное для производителя (Vendor Specific)";
        default:   return "неизвестный класс";
    }
}

void printdev(libusb_device *dev) {
    libusb_device_descriptor desc;
    
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
        cerr << "Ошибка: дескриптор не получен." << endl;
        return;
    }

    int displayClass = desc.bDeviceClass;
    if (displayClass == 0) {
        libusb_config_descriptor *config;
        if (libusb_get_config_descriptor(dev, 0, &config) == LIBUSB_SUCCESS) {
            if (config->bNumInterfaces > 0 && config->interface[0].num_altsetting > 0) {
                displayClass = config->interface[0].altsetting[0].bInterfaceClass;
            }
            libusb_free_config_descriptor(config); 
        }
    }

    string manufacturer = "";
    string product = "";
    string serial = "Отсутствует";

    libusb_device_handle *handle = nullptr;
    
    if (libusb_open(dev, &handle) == LIBUSB_SUCCESS && handle != nullptr) {
        unsigned char string_data[256];
        
        if (desc.iManufacturer > 0 && libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string_data, sizeof(string_data)) > 0) {
            manufacturer = string((char*)string_data);
        }

        if (desc.iProduct > 0 && libusb_get_string_descriptor_ascii(handle, desc.iProduct, string_data, sizeof(string_data)) > 0) {
            product = string((char*)string_data);
        }

        if (desc.iSerialNumber > 0) {
            if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string_data, sizeof(string_data)) > 0) {
                serial = string((char*)string_data);
            } else {
                serial = "[Ошибка чтения]";
            }
        }
        
        libusb_close(handle); 
    } else {
        if (desc.iSerialNumber > 0) serial = "[Нет прав доступа]";
    }

    cout << "========================================================================\n";
    
    cout << "Класс:  " << setfill('0') << setw(2) << hex << displayClass << dec 
         << " - " << getClassName(displayClass) << endl;

    cout << "Пр-во:  ";
    if (!manufacturer.empty()) cout << manufacturer;
    else cout << "VID: 0x" << setfill('0') << setw(4) << hex << desc.idVendor << dec;
    cout << endl;

    cout << "Устр-во:";
    if (!product.empty()) cout << " " << product;
    else cout << " PID: 0x" << setfill('0') << setw(4) << hex << desc.idProduct << dec;
    cout << endl;

    cout << "SN:     " << serial << endl;
}

int main() {
    libusb_device **devs; 
    libusb_context *ctx = nullptr; 
    
    if (libusb_init(&ctx) < 0) {
        cerr << "Ошибка инициализации libusb." << endl;
        return 1;
    }

    libusb_set_option(ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);

    ssize_t cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        cerr << "Ошибка получения списка устройств." << endl;
        libusb_exit(ctx);
        return 1;
    }

    cout << "Найдено устройств: " << cnt << endl;

    for (ssize_t i = 0; i < cnt; i++) {
        printdev(devs[i]);
    }
    cout << "========================================================================\n";

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);

    return 0;
}

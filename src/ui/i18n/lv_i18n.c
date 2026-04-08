#include "./lv_i18n.h"


////////////////////////////////////////////////////////////////////////////////
// Define plural operands
// http://unicode.org/reports/tr35/tr35-numbers.html#Operands

// Integer version, simplified

#define UNUSED(x) (void)(x)

static inline uint32_t op_n(int32_t val) { return (uint32_t)(val < 0 ? -val : val); }
static inline uint32_t op_i(uint32_t val) { return val; }
// always zero, when decimal part not exists.
static inline uint32_t op_v(uint32_t val) { UNUSED(val); return 0;}
static inline uint32_t op_w(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_f(uint32_t val) { UNUSED(val); return 0; }
static inline uint32_t op_t(uint32_t val) { UNUSED(val); return 0; }

static lv_i18n_phrase_t en_singulars[] = {
    {"config", "Config"},
    {"speed_ctrl", "Speed ctrl"},
    {"firmware", "Firmware"},
    {"system", "System"},
    {"save", "Save"},
    {"load", "Load"},
    {"unsaved", "unsaved*"},
    {"config_load_first", "Please load settings first"},
    {"config_save_confirm", "Save settings to AM32?"},
    {"config_load_unsaved_warning", "You have unsaved changes. Load settings from AM32? Unsaved changes will be lost."},
    {"config_back_unsaved_warning", "You have unsaved changes. Are you sure to leave without saving?"},
    {"config_read_failed", "Failed to read parameters from AM32. Check power or connection."},
    {"config_write_failed", "Failed to write parameters to AM32. Check power or connection."},
    {"speed_control", "Speed control"},
    {"smooth", "Smooth"},
    {"disable_smooth_warning", "Sudden speed changes may damage the ESC. Confirm to disable smooth mode?"},
    {"speed_format", "Speed: %lu%%"},
    {"widget_color", "Widget color"},
    {"lcd_brightness", "LCD Brightness"},
    {"system_widget_color", "Widget color"},
    {"system_lcd_brightness", "LCD brightness"},
    {"system_factory_reset", "Factory Reset"},
    {"system_language", "Language"},
    {"system_about", "About"},
    {"factory_reset_confirm", "Are you sure to reset to factory settings?"},
    {"language_english", "English"},
    {"language_chinese", "中文(简体)"},
    {"language_korean", "한국어"},
    {"about_hardware_version_format", "Hardware version: %s"},
    {"about_software_version_format", "Software version: %s"},
    {"about_build_time_format", "Build time: %s"},
    {"about_uid_format", "UID: %08lX %08lX %08lX"},
    {"select_file", "Select file:"},
    {"success", "success"},
    {"update_firmware", "Update firmware"},
    {"no_hex_files_found", "No *.hex files found"},
    {"firmware_updating_wait", "Firmware updating, please wait..."},
    {"error_unknown", "unknown error"},
    {"error_invalid_path", "Invalid file path"},
    {"error_invalid_path_length", "Invalid file path length"},
    {"error_hex_start_address", "Hex file start address is invalid"},
    {"error_open_hex_failed", "Failed to open hex file"},
    {"error_check_hex_failed", "Check hex failed(%ld)"},
    {"error_write_flash", "failed to write flash at address %08lX"},
    {"error_read_hex", "error reading hex file(%ld)"},
    {"car_type_reverse_braking", "Car type reverse braking"},
    {"brake_on_stop", "Brake on stop"},
    {"brake_on_stop_option_off", "Off"},
    {"brake_on_stop_option_brake", "Brake on stop"},
    {"brake_on_stop_option_active", "Active brake"},
    {"off", "Off"},
    {"brake_strength", "Brake strength"},
    {"running_brake_level", "Running brake level"},
    {"active_brake_power", "Active brake power"},
    {"duty_cycle", "%u%% duty cycle"},
    {"protocol", "Protocol:"},
    {"protocol_option_auto", "AUTO"},
    {"protocol_option_dshot", "Dshot"},
    {"protocol_option_servo", "Servo"},
    {"protocol_option_serial", "Serial"},
    {"protocol_option_edt_arm", "EDT ARM"},
    {"motor_direction_reverse", "Motor direction reverse"},
    {"disable_stick_calibration", "Disable Stick Calibration"},
    {"stuck_rotor_protection", "Stuck Rotor Protection"},
    {"stall_protection", "Stall protection"},
    {"use_hall_sensors", "Use hall sensors"},
    {"telemetry_30ms", "30ms interval telemetry"},
    {"complementary_pwm", "Complementary PWM"},
    {"auto_timing_advance", "Auto timing advance"},
    {"timing_advance", "Timing Advance"},
    {"startup_power", "Startup power"},
    {"motor_kv", "Motor KV"},
    {"motor_poles", "Motor poles"},
    {"beeper_volume", "Beeper volume"},
    {"ramp_rate", "Ramp rate"},
    {"ramp_rate_format", "%u.%u%% duty cycle per ms"},
    {"pwm_type", "PWM Type"},
    {"pwm_type_option_fixed", "Fixed"},
    {"pwm_type_option_variable", "Variable"},
    {"pwm_type_option_by_rpm", "by RPM"},
    {"pwm_frequency", "PWM Frequency"},
    {"minimum_duty_cycle", "Minimum duty cycle"},
    {"low_voltage_cut_off", "Low Voltage Cut Off"},
    {"lvco_option_off", "Off"},
    {"lvco_option_cell_based", "Cell based"},
    {"lvco_option_absolute", "Absolute"},
    {"temperature_limit", "Temperature limit"},
    {"current_limit", "Current limit"},
    {"disabled", "Disabled"},
    {"low_voltage_cut_off_threshold", "Low voltage cut off threshold"},
    {"absolute_voltage_cutoff", "Absolute voltage cutoff"},
    {"editable_if_current_limit", "Editable if current limit works"},
    {"current_p", "Current P"},
    {"current_i", "Current I"},
    {"current_d", "Current D"},
    {"low_threshold", "Low threshold"},
    {"high_threshold", "High threshold"},
    {"neutral", "Neutral"},
    {"dead_band", "Dead band"},
    {"sinusoidal_startup", "Sinusoidal startup"},
    {"sine_mode_range", "Sine mode range"},
    {"sine_mode_power", "Sine mode power"},
    {"bootloader", "Bootloader"},
    {"pin", "PIN:"},
    {"pin_format", "PIN: P%c%c"},
    {"version", "Version:"},
    {"version_format", "Version: %d"},
    {"flash_colon", "Flash:"},
    {"flash_colon_format", "Flash: 0x%02X"},
    {"mcu", "MCU"},
    {"type", "Type:"},
    {"type_format", "Type: %s"},
    {"eeprom", "EEPROM:"},
    {"eeprom_format", "EEPROM: v%d"},
    {"firmware_info", "Firmware"},
    {"name", "Name:"},
    {"name_format", "Name: %s"},
    {"version_firmware", "Version:"},
    {"version_firmware_format", "Version: %u.%02u"},
    {"need_startup", "(Need startup)"},
    {"default_settings", "Default Settings"},
    {"default_settings_confirm", "Are you sure to overwrite the current config with default settings?"},
    {"version_default", "Version: default"},
    {NULL, NULL} // End mark
};



static uint8_t en_plural_fn(int32_t num)
{
    uint32_t n = op_n(num); UNUSED(n);
    uint32_t i = op_i(n); UNUSED(i);
    uint32_t v = op_v(n); UNUSED(v);

    if ((i == 1 && v == 0)) return LV_I18N_PLURAL_TYPE_ONE;
    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t en_lang = {
    .locale_name = "en",
    .singulars = en_singulars,

    .locale_plural_fn = en_plural_fn
};

static lv_i18n_phrase_t zh_cn_singulars[] = {
    {"config", "配置"},
    {"speed_ctrl", "速度控制"},
    {"firmware", "固件"},
    {"system", "系统"},
    {"save", "保存"},
    {"load", "读取"},
    {"unsaved", "未保存*"},
    {"config_load_first", "请先读取设置"},
    {"config_save_confirm", "保存设置到AM32？"},
    {"config_load_unsaved_warning", "您有未保存的更改。从AM32读取设置？未保存的更改将丢失。"},
    {"config_back_unsaved_warning", "您有未保存的更改。确定不保存就离开吗？"},
    {"config_read_failed", "从AM32读取参数失败。请检查电源或连接。"},
    {"config_write_failed", "向AM32写入参数失败。请检查电源或连接。"},
    {"speed_control", "速度控制"},
    {"smooth", "平滑"},
    {"disable_smooth_warning", "突然的速度变化可能损坏电调。确认禁用平滑模式？"},
    {"speed_format", "速度：%lu%%"},
    {"widget_color", "控件颜色"},
    {"lcd_brightness", "LCD亮度"},
    {"system_widget_color", "控件颜色"},
    {"system_lcd_brightness", "LCD亮度"},
    {"system_factory_reset", "恢复出厂设置"},
    {"system_language", "语言"},
    {"system_about", "关于"},
    {"factory_reset_confirm", "确定要恢复出厂设置吗？"},
    {"language_english", "English"},
    {"language_chinese", "中文(简体)"},
    {"language_korean", "한국어"},
    {"about_hardware_version_format", "硬件版本：%s"},
    {"about_software_version_format", "软件版本：%s"},
    {"about_build_time_format", "编译时间：%s"},
    {"about_uid_format", "UID：%08lX %08lX %08lX"},
    {"select_file", "选择文件："},
    {"success", "成功"},
    {"update_firmware", "更新固件"},
    {"no_hex_files_found", "未找到 *.hex 文件"},
    {"firmware_updating_wait", "固件正在更新，请稍候..."},
    {"error_unknown", "未知错误"},
    {"error_invalid_path", "无效的文件路径"},
    {"error_invalid_path_length", "文件路径长度无效"},
    {"error_hex_start_address", "Hex文件起始地址无效"},
    {"error_open_hex_failed", "打开Hex文件失败"},
    {"error_check_hex_failed", "检查Hex失败(%ld)"},
    {"error_write_flash", "写入Flash失败，地址 %08lX"},
    {"error_read_hex", "读取Hex文件错误(%ld)"},
    {"car_type_reverse_braking", "车型反向刹车"},
    {"brake_on_stop", "停止时刹车"},
    {"brake_on_stop_option_off", "关闭"},
    {"brake_on_stop_option_brake", "停止刹车"},
    {"brake_on_stop_option_active", "主动刹车"},
    {"off", "关闭"},
    {"brake_strength", "刹车强度"},
    {"running_brake_level", "运行刹车等级"},
    {"active_brake_power", "主动刹车功率"},
    {"duty_cycle", "%u%% 占空比"},
    {"protocol", "协议："},
    {"protocol_option_auto", "AUTO"},
    {"protocol_option_dshot", "Dshot"},
    {"protocol_option_servo", "Servo"},
    {"protocol_option_serial", "Serial"},
    {"protocol_option_edt_arm", "EDT ARM"},
    {"motor_direction_reverse", "电机反向"},
    {"disable_stick_calibration", "禁用摇杆校准"},
    {"stuck_rotor_protection", "堵转保护"},
    {"stall_protection", "失速保护"},
    {"use_hall_sensors", "使用霍尔传感器"},
    {"telemetry_30ms", "30ms间隔遥测"},
    {"complementary_pwm", "互补PWM"},
    {"auto_timing_advance", "自动提前角"},
    {"timing_advance", "提前角"},
    {"startup_power", "启动功率"},
    {"motor_kv", "电机KV"},
    {"motor_poles", "电机极数"},
    {"beeper_volume", "电机蜂鸣音量"},
    {"ramp_rate", "变化速率"},
    {"ramp_rate_format", "%u.%u%% 占空比/ms"},
    {"pwm_type", "PWM类型"},
    {"pwm_type_option_fixed", "固定"},
    {"pwm_type_option_variable", "可变"},
    {"pwm_type_option_by_rpm", "按RPM"},
    {"pwm_frequency", "PWM频率"},
    {"minimum_duty_cycle", "最小占空比"},
    {"low_voltage_cut_off", "低压保护"},
    {"lvco_option_off", "关闭"},
    {"lvco_option_cell_based", "基于电芯"},
    {"lvco_option_absolute", "绝对值"},
    {"temperature_limit", "温度限制保护"},
    {"current_limit", "电流限制保护"},
    {"disabled", "已禁用"},
    {"low_voltage_cut_off_threshold", "低压保护阈值"},
    {"absolute_voltage_cutoff", "绝对电压截止"},
    {"editable_if_current_limit", "当电流限制保护开启时,下列参数有效"},
    {"current_p", "电流P"},
    {"current_i", "电流I"},
    {"current_d", "电流D"},
    {"low_threshold", "输入PWM启动阈值"},
    {"high_threshold", "输入PWM最高阈值"},
    {"neutral", "输入PWM中位"},
    {"dead_band", "输入PWM死区"},
    {"sinusoidal_startup", "正弦启动"},
    {"sine_mode_range", "正弦模式范围"},
    {"sine_mode_power", "正弦模式功率"},
    {"bootloader", "Bootloader"},
    {"pin", "PIN："},
    {"pin_format", "PIN: P%c%c"},
    {"version", "版本："},
    {"version_format", "版本：%d"},
    {"flash_colon", "Flash:"},
    {"flash_colon_format", "Flash: 0x%02X"},
    {"mcu", "MCU"},
    {"type", "类型："},
    {"type_format", "类型：%s"},
    {"eeprom", "EEPROM："},
    {"eeprom_format", "EEPROM: v%d"},
    {"firmware_info", "固件"},
    {"name", "名称："},
    {"name_format", "名称：%s"},
    {"version_firmware", "版本："},
    {"version_firmware_format", "版本：%u.%02u"},
    {"need_startup", "(需要启动)"},
    {"default_settings", "默认设置"},
    {"default_settings_confirm", "确定要用默认设置覆盖当前配置吗？"},
    {"version_default", "版本：默认"},
    {NULL, NULL} // End mark
};



static uint8_t zh_cn_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t zh_cn_lang = {
    .locale_name = "zh-cn",
    .singulars = zh_cn_singulars,

    .locale_plural_fn = zh_cn_plural_fn
};

static lv_i18n_phrase_t ko_singulars[] = {
    {"config", "설정"},
    {"speed_ctrl", "속도 제어"},
    {"firmware", "펌웨어"},
    {"system", "시스템"},
    {"save", "저장"},
    {"load", "불러오기"},
    {"unsaved", "저장되지 않음*"},
    {"config_load_first", "먼저 설정을 불러오세요"},
    {"config_save_confirm", "설정을 AM32에 저장할까요?"},
    {"config_load_unsaved_warning", "저장되지 않은 변경이 있습니다. AM32에서 설정을 불러오면 변경 내용이 사라집니다."},
    {"config_back_unsaved_warning", "저장되지 않은 변경이 있습니다. 저장하지 않고 나가시겠습니까?"},
    {"config_read_failed", "AM32에서 매개변수 읽기 실패. 전원 또는 연결을 확인하세요."},
    {"config_write_failed", "AM32에 매개변수 쓰기 실패. 전원 또는 연결을 확인하세요."},
    {"speed_control", "속도 제어"},
    {"smooth", "부드럽게"},
    {"disable_smooth_warning", "급격한 속도 변화는 ESC를 손상시킬 수 있습니다. 부드러운 모드를 비활성화할까요?"},
    {"speed_format", "속도: %lu%%"},
    {"widget_color", "위젯 색상"},
    {"lcd_brightness", "LCD 밝기"},
    {"system_widget_color", "위젯 색상"},
    {"system_lcd_brightness", "LCD 밝기"},
    {"system_factory_reset", "공장 초기화"},
    {"system_language", "언어"},
    {"system_about", "정보"},
    {"factory_reset_confirm", "공장 초기화하시겠습니까?"},
    {"language_english", "English"},
    {"language_chinese", "中文(简体)"},
    {"language_korean", "한국어"},
    {"about_hardware_version_format", "하드웨어 버전: %s"},
    {"about_software_version_format", "소프트웨어 버전: %s"},
    {"about_build_time_format", "빌드 시간: %s"},
    {"about_uid_format", "UID: %08lX %08lX %08lX"},
    {"select_file", "파일 선택:"},
    {"success", "성공"},
    {"update_firmware", "펌웨어 업데이트"},
    {"no_hex_files_found", "*.hex 파일이 없습니다"},
    {"firmware_updating_wait", "펌웨어 업데이트 중입니다. 잠시 기다려 주세요..."},
    {"error_unknown", "알 수 없는 오류"},
    {"error_invalid_path", "잘못된 파일 경로"},
    {"error_invalid_path_length", "파일 경로 길이가 올바르지 않습니다"},
    {"error_hex_start_address", "Hex 파일 시작 주소가 올바르지 않습니다"},
    {"error_open_hex_failed", "Hex 파일 열기 실패"},
    {"error_check_hex_failed", "Hex 검사 실패(%ld)"},
    {"error_write_flash", "Flash 쓰기 실패 주소 %08lX"},
    {"error_read_hex", "Hex 파일 읽기 오류(%ld)"},
    {"car_type_reverse_braking", "차량 타입 역방향 브레이크"},
    {"brake_on_stop", "정지 시 브레이크"},
    {"brake_on_stop_option_off", "끔"},
    {"brake_on_stop_option_brake", "정지 브레이크"},
    {"brake_on_stop_option_active", "액티브 브레이크"},
    {"off", "끔"},
    {"brake_strength", "브레이크 강도"},
    {"running_brake_level", "주행 브레이크 레벨"},
    {"active_brake_power", "액티브 브레이크 파워"},
    {"duty_cycle", "%u%% 듀티 사이클"},
    {"protocol", "프로토콜:"},
    {"protocol_option_auto", "AUTO"},
    {"protocol_option_dshot", "Dshot"},
    {"protocol_option_servo", "Servo"},
    {"protocol_option_serial", "Serial"},
    {"protocol_option_edt_arm", "EDT ARM"},
    {"motor_direction_reverse", "모터 역방향"},
    {"disable_stick_calibration", "스틱 캘리브레이션 비활성화"},
    {"stuck_rotor_protection", "로터 고착 보호"},
    {"stall_protection", "스톨 보호"},
    {"use_hall_sensors", "홀 센서 사용"},
    {"telemetry_30ms", "30ms 간격 텔레메트리"},
    {"complementary_pwm", "보완 PWM"},
    {"auto_timing_advance", "자동 타이밍 어드밴스"},
    {"timing_advance", "타이밍 어드밴스"},
    {"startup_power", "시동 전력"},
    {"motor_kv", "모터 KV"},
    {"motor_poles", "모터 극수"},
    {"beeper_volume", "비퍼 음량"},
    {"ramp_rate", "램프 속도"},
    {"ramp_rate_format", "%u.%u%% 듀티 사이클/ms"},
    {"pwm_type", "PWM 유형"},
    {"pwm_type_option_fixed", "고정"},
    {"pwm_type_option_variable", "가변"},
    {"pwm_type_option_by_rpm", "RPM 기준"},
    {"pwm_frequency", "PWM 주파수"},
    {"minimum_duty_cycle", "최소 듀티 사이클"},
    {"low_voltage_cut_off", "저전압 차단"},
    {"lvco_option_off", "끔"},
    {"lvco_option_cell_based", "셀 기반"},
    {"lvco_option_absolute", "절대값"},
    {"temperature_limit", "온도 제한"},
    {"current_limit", "전류 제한"},
    {"disabled", "비활성화됨"},
    {"low_voltage_cut_off_threshold", "저전압 차단 임계값"},
    {"absolute_voltage_cutoff", "절대 전압 차단"},
    {"editable_if_current_limit", "전류 제한이 동작할 때만 편집 가능"},
    {"current_p", "전류 P"},
    {"current_i", "전류 I"},
    {"current_d", "전류 D"},
    {"low_threshold", "낮은 임계값"},
    {"high_threshold", "높은 임계값"},
    {"neutral", "중립"},
    {"dead_band", "데드밴드"},
    {"sinusoidal_startup", "사인파 시작"},
    {"sine_mode_range", "사인 모드 범위"},
    {"sine_mode_power", "사인 모드 파워"},
    {"bootloader", "부트로더"},
    {"pin", "PIN:"},
    {"pin_format", "PIN: P%c%c"},
    {"version", "버전:"},
    {"version_format", "버전: %d"},
    {"flash_colon", "Flash:"},
    {"flash_colon_format", "Flash: 0x%02X"},
    {"mcu", "MCU"},
    {"type", "타입:"},
    {"type_format", "타입: %s"},
    {"eeprom", "EEPROM:"},
    {"eeprom_format", "EEPROM: v%d"},
    {"firmware_info", "펌웨어"},
    {"name", "이름:"},
    {"name_format", "이름: %s"},
    {"version_firmware", "버전:"},
    {"version_firmware_format", "버전: %u.%02u"},
    {"need_startup", "(시동 필요)"},
    {"default_settings", "기본 설정"},
    {"default_settings_confirm", "기본 설정으로 현재 구성을 덮어쓸까요?"},
    {"version_default", "버전: 기본"},
    {NULL, NULL} // End mark
};



static uint8_t ko_plural_fn(int32_t num)
{



    return LV_I18N_PLURAL_TYPE_OTHER;
}

static const lv_i18n_lang_t ko_lang = {
    .locale_name = "ko",
    .singulars = ko_singulars,

    .locale_plural_fn = ko_plural_fn
};

const lv_i18n_language_pack_t lv_i18n_language_pack[] = {
    &en_lang,
    &zh_cn_lang,
    &ko_lang,
    NULL // End mark
};

////////////////////////////////////////////////////////////////////////////////


// Internal state
static const lv_i18n_language_pack_t * current_lang_pack;
static const lv_i18n_lang_t * current_lang;


/**
 * Reset internal state. For testing.
 */
void __lv_i18n_reset(void)
{
    current_lang_pack = NULL;
    current_lang = NULL;
}

/**
 * Set the languages for internationalization
 * @param langs pointer to the array of languages. (Last element has to be `NULL`)
 */
int lv_i18n_init(const lv_i18n_language_pack_t * langs)
{
    if(langs == NULL) return -1;
    if(langs[0] == NULL) return -1;

    current_lang_pack = langs;
    current_lang = langs[0];     /*Automatically select the first language*/
    return 0;
}

/**
 * Change the localization (language)
 * @param l_name name of the translation locale to use. E.g. "en-GB"
 */
int lv_i18n_set_locale(const char * l_name)
{
    if(current_lang_pack == NULL) return -1;

    uint16_t i;

    for(i = 0; current_lang_pack[i] != NULL; i++) {
        // Found -> finish
        if(strcmp(current_lang_pack[i]->locale_name, l_name) == 0) {
            current_lang = current_lang_pack[i];
            return 0;
        }
    }

    return -1;
}


static const char * __lv_i18n_get_text_core(lv_i18n_phrase_t * trans, const char * msg_id)
{
    uint16_t i;
    for(i = 0; trans[i].msg_id != NULL; i++) {
        if(strcmp(trans[i].msg_id, msg_id) == 0) {
            /*The msg_id has found. Check the translation*/
            if(trans[i].translation) return trans[i].translation;
        }
    }

    return NULL;
}


/**
 * Get the translation from a message ID
 * @param msg_id message ID
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text(const char * msg_id)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;

    // Search in current locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->singulars != NULL) {
        txt = __lv_i18n_get_text_core(lang->singulars, msg_id);
        if (txt != NULL) return txt;
    }

    return msg_id;
}

/**
 * Get the translation from a message ID and apply the language's plural rule to get correct form
 * @param msg_id message ID
 * @param num an integer to select the correct plural form
 * @return the translation of `msg_id` on the set local
 */
const char * lv_i18n_get_text_plural(const char * msg_id, int32_t num)
{
    if(current_lang == NULL) return msg_id;

    const lv_i18n_lang_t * lang = current_lang;
    const void * txt;
    lv_i18n_plural_type_t ptype;

    // Search in current locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    // Try to fallback
    if(lang == current_lang_pack[0]) return msg_id;
    lang = current_lang_pack[0];

    // Repeat search for default locale
    if(lang->locale_plural_fn != NULL) {
        ptype = lang->locale_plural_fn(num);

        if(lang->plurals[ptype] != NULL) {
            txt = __lv_i18n_get_text_core(lang->plurals[ptype], msg_id);
            if (txt != NULL) return txt;
        }
    }

    return msg_id;
}

/**
 * Get the name of the currently used locale.
 * @return name of the currently used locale. E.g. "en-GB"
 */
const char * lv_i18n_get_current_locale(void)
{
    if(!current_lang) return NULL;
    return current_lang->locale_name;
}

//
// Created by hanji on 2025/2/9.
//

#include "settings.h"
#include "config.h"

#include "Includes.h"

#define DEBUG 0

static const char* GetEnvVar(const char *name)
{
	return getenv(name);
}

static int GetEnvVarInt(const char *name,int *i,int def)
{
	const char *s=GetEnvVar(name);
	*i=s ? atoi(s) : def;
	return s!=NULL;
}


global_settings_t global_settings;

void init_settings() {
#if defined(__APPLE__)
    global_settings.angle = AngleMode::Disabled;
    global_settings.ignore_error = IgnoreErrorLevel::Partial;
    global_settings.ext_gl43 = false;
    global_settings.ext_compute_shader = false;
    global_settings.max_glsl_cache_size = 30 * 1024 * 1024;
    global_settings.multidraw_mode = multidraw_mode_t::DrawElements;
    global_settings.angle_depth_clear_fix_mode = AngleDepthClearFixMode::Disabled;
    global_settings.ext_direct_state_access = true;
    global_settings.custom_gl_version = {0, 0, 0}; // will go default
    global_settings.fsr1_setting = FSR1_Quality_Preset::Disabled;
    global_settings.hide_mg_env_level = HideMGEnvLevel::Disabled;

#else

    int success = initialized;
    if (!success) {
        success = config_refresh();
        if (!success) {
            MGLOG_W("Failed to load config. Use default config.");
        }
    }

    AngleConfig angleConfig =
        success ? static_cast<AngleConfig>(config_get_int("enableANGLE")) : AngleConfig::DisableIfPossible;
    NoErrorConfig noErrorConfig =
        success ? static_cast<NoErrorConfig>(config_get_int("enableNoError")) : NoErrorConfig::Auto;
    bool enableExtGL43 = success ? (config_get_int("enableExtGL43") > 0) : false;
    bool enableExtComputeShader = success ? (config_get_int("enableExtComputeShader") > 0) : false;
    bool enableExtTimerQuery = success ? (config_get_int("enableExtTimerQuery") > 0) : false;
    bool enableExtDirectStateAccess = success ? (config_get_int("enableExtDirectStateAccess") > 0) : false;
    AngleDepthClearFixMode angleDepthClearFixMode =
        success ? static_cast<AngleDepthClearFixMode>(config_get_int("angleDepthClearFixMode"))
                : AngleDepthClearFixMode::Disabled;
    int customGLVersionInt = success ? config_get_int("customGLVersion") : DEFAULT_GL_VERSION;
    FSR1_Quality_Preset fsr1Setting =
        success ? static_cast<FSR1_Quality_Preset>(config_get_int("fsr1Setting")) : FSR1_Quality_Preset::Disabled;
    HideMGEnvLevel hideMGEnvLevel =
        success ? static_cast<HideMGEnvLevel>(config_get_int("hideMGEnvLevel")) : HideMGEnvLevel::Disabled;

    if (customGLVersionInt < 0) {
        customGLVersionInt = 0;
    }

    size_t maxGlslCacheSize = 0;
    if (config_get_int("maxGlslCacheSize") > 0) {
        maxGlslCacheSize = success ? config_get_int("maxGlslCacheSize") * 1024 * 1024 : 0;
    }

    if (static_cast<int>(angleConfig) < 0 || static_cast<int>(angleConfig) > 3) {
        angleConfig = AngleConfig::EnableIfPossible;
    }
    if (static_cast<int>(noErrorConfig) < 0 || static_cast<int>(noErrorConfig) > 3) {
        noErrorConfig = NoErrorConfig::Auto;
    }
    if (static_cast<int>(angleDepthClearFixMode) < 0 ||
        static_cast<int>(angleDepthClearFixMode) >= static_cast<int>(AngleDepthClearFixMode::MaxValue)) {
        angleDepthClearFixMode = AngleDepthClearFixMode::Disabled;
    }
    if (static_cast<int>(fsr1Setting) < 0 ||
        static_cast<int>(fsr1Setting) >= static_cast<int>(FSR1_Quality_Preset::MaxValue)) {
        fsr1Setting = FSR1_Quality_Preset::Disabled;
    }
    if (static_cast<int>(hideMGEnvLevel) < 0 ||
        static_cast<int>(hideMGEnvLevel) >= static_cast<int>(HideMGEnvLevel::MaxValue)) {
        hideMGEnvLevel = HideMGEnvLevel::Disabled;
    }

    Version customGLVersion(customGLVersionInt);

    int isInPluginApp = 0;
    GetEnvVarInt("MG_PLUGIN_STATUS", &isInPluginApp, 0);
    int fclVersion = 0;
    GetEnvVarInt("FCL_VERSION_CODE", &fclVersion, 0);
    int zlVersion = 0;
    GetEnvVarInt("ZALITH_VERSION_CODE", &zlVersion, 0);
    int pgwVersion = 0;
    GetEnvVarInt("PGW_VERSION_CODE", &pgwVersion, 0);

    MGLOG_D("MG_DIR_PATH = %s", mg_directory_path ? mg_directory_path : "(default)");

    AngleMode finalAngleMode = AngleMode::Disabled;

    bool isANGLESupported = true;

    switch (angleConfig) {
    case AngleConfig::ForceDisable:
        finalAngleMode = AngleMode::Disabled;
        MGLOG_D("ANGLE: Force disabled");
        break;

    case AngleConfig::ForceEnable:
        finalAngleMode = AngleMode::Enabled;
        MGLOG_D("ANGLE: Force enabled");
        break;

    case AngleConfig::EnableIfPossible: {
        finalAngleMode = isANGLESupported ? AngleMode::Enabled : AngleMode::Disabled;
        MGLOG_D("ANGLE: Conditionally %s", (finalAngleMode == AngleMode::Enabled) ? "enabled" : "disabled");
        break;
    }

    case AngleConfig::DisableIfPossible:
    default:
        finalAngleMode = AngleMode::Disabled;
        break;
    }

    global_settings.angle = finalAngleMode;
    MGLOG_D("Final ANGLE setting: %d", static_cast<int>(global_settings.angle));
    global_settings.buffer_coherent_as_flush = (global_settings.angle == AngleMode::Disabled);

    if (global_settings.angle == AngleMode::Enabled) {
        // setenv("LIBGL_GLES", "libGLESv2_angle.so", 1);
        // setenv("LIBGL_EGL", "libEGL_angle.so", 1);
        setenv("LIBGL_EGL", "libEGL_angle.so", 1);
    } else if (angleConfig == AngleConfig::EnableIfPossible) {
		setenv("LIBGL_EGL", "libEGL_mesa.so", 1);
	}

    switch (noErrorConfig) {
    case NoErrorConfig::Level1:
        global_settings.ignore_error = IgnoreErrorLevel::Partial;
        MGLOG_D("Error ignoring: Level 1 (Partial)");
        break;

    case NoErrorConfig::Level2:
        global_settings.ignore_error = IgnoreErrorLevel::Full;
        MGLOG_D("Error ignoring: Level 2 (Full)");
        break;

    case NoErrorConfig::Auto:
    case NoErrorConfig::Disable:
    default:
        global_settings.ignore_error = IgnoreErrorLevel::None;
        MGLOG_D("Error ignoring: Disabled");
        break;
    }

    global_settings.ext_gl43 = enableExtGL43;
    global_settings.ext_compute_shader = enableExtComputeShader;
    global_settings.ext_timer_query = enableExtTimerQuery;
    global_settings.ext_direct_state_access = enableExtDirectStateAccess;
    global_settings.max_glsl_cache_size = maxGlslCacheSize;
    global_settings.angle_depth_clear_fix_mode = angleDepthClearFixMode;
    global_settings.custom_gl_version = customGLVersion;
    global_settings.fsr1_setting = fsr1Setting;
    global_settings.hide_mg_env_level = hideMGEnvLevel;
#endif

    MGLOG_D("[MobileGL] Setting: enableAngle                 = %s",
          global_settings.angle == AngleMode::Enabled ? "true" : "false");
    MGLOG_D("[MobileGL] Setting: ignoreError                 = %i", static_cast<int>(global_settings.ignore_error));
    MGLOG_D("[MobileGL] Setting: enableExtComputeShader is unsupported.");
    MGLOG_D("[MobileGL] Setting: enableExtGL43               = %s", global_settings.ext_gl43 ? "true" : "false");
    MGLOG_D("[MobileGL] Setting: enableExtTimerQuery is unsupported.");
    MGLOG_D("[MobileGL] Setting: enableExtDirectStateAccess is unsupported.");
    MGLOG_D("[MobileGL] Setting: maxGlslCacheSize is unsupported.");
    MGLOG_D("[MobileGL] Setting: angleDepthClearFixMode is unsupported.");
    MGLOG_D("[MobileGL] Setting: bufferCoherentAsFlush is unsupported.");
    if (global_settings.custom_gl_version.isEmpty()) {
        MGLOG_D("[MobileGL] Setting: customGLVersion is unsupported.");
    } else {
        MGLOG_D("[MobileGL] Setting: customGLVersion is unsupported.");
    }
    MGLOG_D("[MobileGL] Setting: fsr1Setting is unsupported.");
    MGLOG_D("[MobileGL] Setting: hideMGEnvLevel is unsupported.");

	if (global_settings.ext_gl43) {
        setenv("LIBGL_GL", "43", 1);
	}

	if (global_settings.ignore_error != IgnoreErrorLevel::None) {
		setenv("MGL_CHEAT_CHECKFRAMEBUFFERSTATUS", "true", 1);
	}

	if (global_settings.ext_compute_shader) {
		setenv("LIBGL_COMPUTE_SHADER", "true", 1);
	}
}

void set_multidraw_setting() { // should be called after init_gles_target()
    multidraw_mode_t multidrawMode = static_cast<multidraw_mode_t>(config_get_int("multidrawMode"));
    if (static_cast<int>(multidrawMode) == -1) {
        multidrawMode = multidraw_mode_t::Auto;
    }
    if (static_cast<int>(multidrawMode) < 0 ||
        static_cast<int>(multidrawMode) >= static_cast<int>(multidraw_mode_t::MaxValue)) {
        multidrawMode = multidraw_mode_t::Auto;
    }
    global_settings.multidraw_mode = multidrawMode;
    std::string draw_mode_str;
    switch (global_settings.multidraw_mode) {
    case multidraw_mode_t::PreferIndirect:
        draw_mode_str = "Indirect";
        break;
    case multidraw_mode_t::PreferBaseVertex:
        draw_mode_str = "Unroll";
        break;
    case multidraw_mode_t::PreferMultidrawIndirect:
        draw_mode_str = "Multidraw indirect";
        break;
    case multidraw_mode_t::DrawElements:
        draw_mode_str = "DrawElements";
        break;
    case multidraw_mode_t::Compute:
        draw_mode_str = "Compute";
        break;
    case multidraw_mode_t::Auto:
        draw_mode_str = "Auto";
        break;
    default:
        draw_mode_str = "(Unknown)";
        global_settings.multidraw_mode = multidraw_mode_t::Auto;
        break;
    }
    MGLOG_D("[MobileGL] Setting: multidrawMode is supported.", /*draw_mode_str.c_str()*/);
}

void init_settings_post() {
    bool multidraw = true;
    bool basevertex = true;
    bool indirect = true;

    switch (global_settings.multidraw_mode) {
    case multidraw_mode_t::PreferIndirect:
        if (indirect) {
			MGLOG_W("LIBGL_MULTIDRAW == Indirect");
			setenv("LIBGL_MULTIDRAW", "Indirect", 1);
            global_settings.multidraw_mode = multidraw_mode_t::PreferIndirect;
        } else if (basevertex) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferBaseVertex;
        } else {
            global_settings.multidraw_mode = multidraw_mode_t::DrawElements;
        }
        break;
    case multidraw_mode_t::PreferBaseVertex:
        if (basevertex) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferBaseVertex;
			setenv("LIBGL_MULTIDRAW", "Vertex", 1);
        } else if (multidraw) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferMultidrawIndirect;
        } else if (indirect) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferIndirect;
        } else {
            global_settings.multidraw_mode = multidraw_mode_t::DrawElements;
        }
        break;
    case multidraw_mode_t::DrawElements:
        global_settings.multidraw_mode = multidraw_mode_t::DrawElements;
        break;
    case multidraw_mode_t::Compute:
        global_settings.multidraw_mode = multidraw_mode_t::Compute;
        break;
    case multidraw_mode_t::Auto:
		break;
    default:
        if (basevertex) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferBaseVertex;
        } else if (multidraw) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferMultidrawIndirect;
        } else if (indirect) {
            global_settings.multidraw_mode = multidraw_mode_t::PreferIndirect;
        } else {
            global_settings.multidraw_mode = multidraw_mode_t::DrawElements;
        }
        break;
    }
}

std::string dump_settings_string(std::string prefix) {
    std::stringstream ss;

    ss << prefix << "Angle: " << (global_settings.angle == AngleMode::Enabled ? "Enabled" : "Disabled") << "\n";
    ss << prefix << "IgnoreError: ";
    switch (global_settings.ignore_error) {
    case IgnoreErrorLevel::None:
        ss << "None";
        break;
    case IgnoreErrorLevel::Partial:
        ss << "Partial";
        break;
    case IgnoreErrorLevel::Full:
        ss << "Full";
        break;
    }
    ss << "\n";

    return ss.str();
}

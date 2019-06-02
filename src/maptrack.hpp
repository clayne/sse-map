/**
 * @file maptrack.hpp
 * @brief Shared interface between files in SSE-MapTrack
 * @internal
 *
 * This file is part of Skyrim SE Map Tracker mod (aka MapTrack).
 *
 *   MapTrack is free software: you can redistribute it and/or modify it
 *   under the terms of the GNU Lesser General Public License as published
 *   by the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   MapTrack is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with MapTrack. If not, see <http://www.gnu.org/licenses/>.
 *
 * @endinternal
 *
 * @ingroup Core
 *
 * @details
 */

#ifndef MAPTRACK_HPP
#define MAPTRACK_HPP

#include <sse-imgui/sse-imgui.h>
#include <utils/winutils.hpp>

#include <d3d11.h>

#define GLM_FORCE_CXX14
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtx/range.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/io.hpp>

#include <array>
#include <vector>
#include <fstream>
#include <string>

//--------------------------------------------------------------------------------------------------

template<class T> static inline glm::vec2
to_vec2 (T const& v) { return glm::vec2 {v.x, v.y}; }
template<class T> static inline ImVec2
to_ImVec2 (T const& v) { return ImVec2 {v.x, v.y}; }

//--------------------------------------------------------------------------------------------------

// skse.cpp

void maptrack_version (int* maj, int* min, int* patch, const char** timestamp);

extern std::ofstream& log ();
extern std::string logfile_path;

extern imgui_api imgui;
extern sseimgui_api sseimgui;

//--------------------------------------------------------------------------------------------------

// fileio.cpp

bool save_settings ();
bool load_settings ();
bool save_track (std::string const& file); ///< Modifies maptrack.track
bool load_track (std::string const& file); ///< Modifies maptrack.track

extern std::string tracks_directory;
extern std::string default_track_file;

//--------------------------------------------------------------------------------------------------

// variables.cpp

bool setup_variables ();

float game_time ();
std::array<float, 3> player_location ();
std::string current_worldspace ();
std::string current_cell ();

void format_game_time (std::string&, const char*, float);
void format_player_location (std::string&, const char*, std::array<float, 3> const&);

/// Simple speed up (is it really needed? or it is for fun only?) through caching

template<int Id>
inline void format_game_time_c (std::string& out, const char* format, float value)
{
    static std::string cached_out;
    static float cached_value = std::numeric_limits<float>::quiet_NaN ();
    if (cached_value != value)
        format_game_time (cached_out, format, cached_value = value);
    out = cached_out;
}

//--------------------------------------------------------------------------------------------------

// render.cpp

struct image_t
{
    std::string file;
    std::uint32_t tint = IM_COL32_WHITE;
    glm::vec4 uv = { 0, 0, 1, 1 };
    ID3D11ShaderResourceView* ref;
};

struct icon_atlas_t
{
    std::string file;
    ID3D11ShaderResourceView* ref;
    std::uint32_t size;       ///< Computed size of #ref (one as the texture is a square)
    float icon_uvsize;        ///< Computed size #icon_size upon the texture #ref
    std::uint32_t icon_size;  ///< Sides size of each icon (0..1] on #ref
    std::uint32_t icon_count; ///< Number of icons in loaded texture.
};

struct icon_t
{
    glm::vec2 src;      ///< Top-left UV from the source texture icon_atlas_t#ref
    std::uint32_t tint;
    glm::vec2 tl, br;   ///< Actual position on the map
};

struct font_t
{
    std::string name;
    float scale;
    float size;
    std::uint32_t color; ///< Only this is tuned by the UI, rest are default init only
    std::string file;
    const char* default_data;
    ImFont* imfont; ///< Actual font with its settings (apart from #color)
};

//--------------------------------------------------------------------------------------------------

/// As reported by player position (xyz) and current game time (t)
typedef glm::vec4 trackpoint_t;

/// Most important stuff for the current running instance
struct maptrack_t
{
    image_t map;
    font_t font;
    glm::vec2 scale, offset; ///< For conversion between texture map and world coords

    icon_atlas_t icon_atlas;
    std::vector<icon_t> icons;

    bool enabled = true;    ///< Is tracking, polling for data is, enabled or not
    int since_dayx = 0;     ///< Show a map track since day X, can't be less than zero actually.
    int last_xdays = 1;     ///< Map track for the last X days, also not less than zero
    float time_point = 1;   ///< Memorize where is the time line slider located
    float update_period;    ///< In seconds, how frequently to poll for data
    float min_distance;     ///< Minimum distance between points, to register a new one

    float track_width;
    std::uint32_t track_color;

    /// Heavy scenario: 60 seconds by 60 minutes by 150 game hours = 540k elements
    std::vector<trackpoint_t> track;
};

extern maptrack_t maptrack;

//--------------------------------------------------------------------------------------------------

#endif


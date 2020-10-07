/**
 * @file lldiskcache.cpp
 * @brief The disk cache implementation.
 *
 * $LicenseInfo:firstyear=2009&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2020, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "linden_common.h"
#include "llassettype.h"
#include "lldir.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <chrono>

#include "lldiskcache.h"

LLDiskCache::LLDiskCache(const std::string cache_dir,
                         const int max_size_bytes,
                         const bool enable_cache_debug_info) :
    mCacheDir(cache_dir),
    mMaxSizeBytes(max_size_bytes),
    mEnableCacheDebugInfo(enable_cache_debug_info)
{
    // the prefix used for cache filenames to disambiguate them from other files
    mCacheFilenamePrefix = "sl_cache";

    // create cache dir if it does not exist
    boost::filesystem::create_directory(cache_dir);
}

void LLDiskCache::purge()
{
    if (mEnableCacheDebugInfo)
    {
        LL_INFOS() << "Total dir size before purge is " << dirFileSize(mCacheDir) << LL_ENDL;
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    typedef std::pair<std::time_t, std::pair<uintmax_t, std::string>> file_info_t;
    std::vector<file_info_t> file_info;

    if (boost::filesystem::is_directory(mCacheDir))
    {
        for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(mCacheDir), {}))
        {
            if (boost::filesystem::is_regular_file(entry))
            {
                if (entry.path().string().find(mCacheFilenamePrefix) != std::string::npos)
                {
                    uintmax_t file_size = boost::filesystem::file_size(entry);
                    const std::string file_path = entry.path().string();
                    const std::time_t file_time = boost::filesystem::last_write_time(entry);

                    file_info.push_back(file_info_t(file_time, { file_size, file_path }));
                }
            }
        }
    }

    std::sort(file_info.begin(), file_info.end(), [](file_info_t& x, file_info_t& y)
    {
        return x.first > y.first;
    });

    LL_INFOS() << "Purging cache to a maximum of " << mMaxSizeBytes << " bytes" << LL_ENDL;

    uintmax_t file_size_total = 0;
    for (file_info_t& entry : file_info)
    {
        file_size_total += entry.second.first;

        std::string action = "";
        if (file_size_total > mMaxSizeBytes)
        {
            action = "DELETE:";
            boost::filesystem::remove(entry.second.second);
        }
        else
        {
            action = "  KEEP:";
        }

        if (mEnableCacheDebugInfo)
        {
            // have to do this because of LL_INFO/LL_END weirdness
            std::ostringstream line;

            line << action << "  ";
            line << entry.first << "  ";
            line << entry.second.first << "  ";
            line << entry.second.second;
            line << " (" << file_size_total << "/" << mMaxSizeBytes << ")";
            LL_INFOS() << line.str() << LL_ENDL;
        }
    }

    if (mEnableCacheDebugInfo)
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto execute_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        LL_INFOS() << "Total dir size after purge is " << dirFileSize(mCacheDir) << LL_ENDL;
        LL_INFOS() << "Cache purge took " << execute_time << " ms to execute for " << file_info.size() << " files" << LL_ENDL;
    }
}

const std::string LLDiskCache::assetTypeToString(LLAssetType::EType at)
{
    /**
     * Make use of the C++17 (or is it 14) feature that allows
     * for inline initialization of an std::map<>
     */
    typedef std::map<LLAssetType::EType, std::string> asset_type_to_name_t;
    asset_type_to_name_t asset_type_to_name =
    {
        { LLAssetType::AT_TEXTURE, "TEXTURE" },
        { LLAssetType::AT_SOUND, "SOUND" },
        { LLAssetType::AT_CALLINGCARD, "CALLINGCARD" },
        { LLAssetType::AT_LANDMARK, "LANDMARK" },
        { LLAssetType::AT_SCRIPT, "SCRIPT" },
        { LLAssetType::AT_CLOTHING, "CLOTHING" },
        { LLAssetType::AT_OBJECT, "OBJECT" },
        { LLAssetType::AT_NOTECARD, "NOTECARD" },
        { LLAssetType::AT_CATEGORY, "CATEGORY" },
        { LLAssetType::AT_LSL_TEXT, "LSL_TEXT" },
        { LLAssetType::AT_LSL_BYTECODE, "LSL_BYTECODE" },
        { LLAssetType::AT_TEXTURE_TGA, "TEXTURE_TGA" },
        { LLAssetType::AT_BODYPART, "BODYPART" },
        { LLAssetType::AT_SOUND_WAV, "SOUND_WAV" },
        { LLAssetType::AT_IMAGE_TGA, "IMAGE_TGA" },
        { LLAssetType::AT_IMAGE_JPEG, "IMAGE_JPEG" },
        { LLAssetType::AT_ANIMATION, "ANIMATION" },
        { LLAssetType::AT_GESTURE, "GESTURE" },
        { LLAssetType::AT_SIMSTATE, "SIMSTATE" },
        { LLAssetType::AT_LINK, "LINK" },
        { LLAssetType::AT_LINK_FOLDER, "LINK_FOLDER" },
        { LLAssetType::AT_MARKETPLACE_FOLDER, "MARKETPLACE_FOLDER" },
        { LLAssetType::AT_WIDGET, "WIDGET" },
        { LLAssetType::AT_PERSON, "PERSON" },
        { LLAssetType::AT_MESH, "MESH" },
        { LLAssetType::AT_SETTINGS, "SETTINGS" },
        { LLAssetType::AT_UNKNOWN, "UNKNOWN" }
    };

    asset_type_to_name_t::iterator iter = asset_type_to_name.find(at);
    if (iter != asset_type_to_name.end())
    {
        return iter->second;
    }

    return std::string("UNKNOWN");
}

const std::string LLDiskCache::metaDataToFilepath(const std::string id,
        LLAssetType::EType at,
        const std::string extra_info)
{
    std::ostringstream file_path;

    file_path << mCacheDir;
    file_path << gDirUtilp->getDirDelimiter();
    file_path << mCacheFilenamePrefix;
    file_path << "_";
    file_path << id;
    file_path << "_";
    file_path << (extra_info.empty() ? "0" : extra_info);
    file_path << "_";
    file_path << assetTypeToString(at);
    file_path << ".asset";

    LL_INFOS() << "filepath.str() = " << file_path.str() << LL_ENDL;

    return file_path.str();
}

/**
 * Update the "last write time" of a file to "now". This must be called whenever a
 * file in the cache is read (not written) so that the last time the file was
 * accessed which is used in the mechanism for purging the cache, is up to date.
 */
void LLDiskCache::updateFileAccessTime(const std::string file_path)
{
    const std::time_t file_time = std::time(nullptr);
    boost::filesystem::last_write_time(file_path, file_time);
}

/**
 * Clear the cache by removing all the files in the cache directory
 * individually. It's important to maintain control of which directory
 * if passed in and not let the user inadvertently (or maliciously) set
 * it to an random location like your project source or OS system directory
 */
void LLDiskCache::clearCache(const std::string cache_dir)
{
    if (boost::filesystem::is_directory(cache_dir))
    {
        for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(cache_dir), {}))
        {
            if (boost::filesystem::is_regular_file(entry))
            {
                if (entry.path().string().find(mCacheFilenamePrefix) != std::string::npos)
                {
                    boost::filesystem::remove(entry);
                }
            }
        }
    }
}

/**
 * Utility function to get the total filesize of all files in a directory. It
 * used to test file extensions to only check cache files but that was removed.
 * There may be a better way that works directly on the folder (similar to
 * right clicking on a folder in the OS and asking for size vs right clicking
 * on all files and adding up manually) but this is very fast - less than 100ms
 * in my testing so, so long as it's not called frequently, it should be okay.
 * Note that's it's only currently used for logging/debugging so if performance
 * is ever an issue, optimizing this or removing it altogether, is an easy win.
 */
uintmax_t LLDiskCache::dirFileSize(const std::string dir)
{
    uintmax_t total_file_size = 0;

    if (boost::filesystem::is_directory(dir))
    {
        for (auto& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(dir), {}))
        {
            if (boost::filesystem::is_regular_file(entry))
            {
                if (entry.path().string().find(mCacheFilenamePrefix) != std::string::npos)
                {
                    total_file_size += boost::filesystem::file_size(entry);
                }
            }
        }
    }

    return total_file_size;
}

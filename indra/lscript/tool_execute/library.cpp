/**
* @file library.cpp
* @brief Non-modifying LSL functions implementations
*
* $LicenseInfo:firstyear=2024&license=viewerlgpl$
* Kyler "Félix" Eastridge
* Copyright (C) 2024, Alchemy Viewer Project.
* Copyright (C) 2010, Linden Research, Inc.
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
* $/LicenseInfo$
*/

#include "library.h"
#include "linden_common.h"
#include "lscript_library.h"
#include "llrand.h"
#include "lltimer.h"
#include "lldate.h"
#include "llmd5.h"
#include <openssl/sha.h> // OpenSSL library for SHA-1 hashing
#include <ctime>

void llSin(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = sin(args[0].mFP);
}

void llCos(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = cos(args[0].mFP);
}

void llTan(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = tan(args[0].mFP);
}

void llAtan2(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = atan2(args[0].mFP, args[1].mFP);
}

void llSqrt(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = sqrt(args[0].mFP);
}

void llPow(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = pow(args[0].mFP, args[1].mFP);
}

void llAbs(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = abs(args[0].mInteger);
}

void llFabs(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = fabs(args[0].mFP);
}

void llFrand(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = ll_frand(args[0].mFP);
}

void llFloor(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = llfloor(args[0].mFP);
}

void llCeil(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = llceil(args[0].mFP);
}

void llRound(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = ll_round(args[0].mFP);
}

void llVecMag(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = args[0].mVec.magVec();
}

void llVecNorm(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    args[0].mVec.normalize();
    retval->mVec = args[0].mVec;
}

void llVecDist(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = dist_vec(args[0].mVec, args[1].mVec);
}

void llRot2Euler(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    args[0].mQuat.getEulerAngles(&retval->mVec.mV[VX], &retval->mVec.mV[VY], &retval->mVec.mV[VZ]);
}

void llEuler2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat.setEulerAngles(args[0].mVec.mV[VX], args[0].mVec.mV[VY], args[0].mVec.mV[VZ]);
}

void llAxes2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat = LLQuaternion(args[0].mVec, args[1].mVec, args[2].mVec);
}

void llRot2Fwd(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(1, 0, 0) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRot2Left(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(0, 1, 0) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRot2Up(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    retval->mVec = LLVector3(0, 0, 1) * args[0].mQuat;
    retval->mVec.normalize();
}

void llRotBetween(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat.shortestArc(args[0].mVec, args[1].mVec);
}

void llGetWallclock(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    struct tm* time = utc_to_pacific_time(time_corrected(), is_daylight_savings());
    retval->mFP = time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec;
}

void llGetSubString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    //TEST: Ensure this works!
    retval->mType = LST_STRING;
    const char *sourceStr = args[0].mString;
    int strLen = strlen(sourceStr);
    int start = args[1].mInteger;
    int end = args[2].mInteger;

    // Handle negative indices
    if (start < 0) start += strLen;
    if (end < 0) end += strLen;

    // Ensure start and end are within bounds
    if (start < 0) start = 0;
    if (end >= strLen) end = strLen - 1;

    // Determine the behavior based on the comparison of start and end
    if (start <= end)
    {
        // Normal substring extraction
        int length = end - start + 1;
        retval->mString = new char[length + 1];
        strncpy(retval->mString, sourceStr + start, length);
        retval->mString[length] = '\0'; // Null-terminate the string
    }
    else
    {
        // Exclusion case: create a string excluding the range from end to start
        int length = strLen - (start - end - 1);
        retval->mString = new char[length + 1];
        strncpy(retval->mString, sourceStr, end + 1);
        strcpy(retval->mString + end + 1, sourceStr + start);
        // Don't need to \0 the string here because we copied that from the other
    }
}

void llDeleteSubString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    const char *sourceStr = args[0].mString;
    int strLen = strlen(sourceStr);
    int start = args[1].mInteger;
    int end = args[2].mInteger;

    // Handle negative indices
    if (start < 0) start += strLen;
    if (end < 0) end += strLen;

    // Ensure start and end are within bounds
    if (start < 0) start = 0;
    if (end >= strLen) end = strLen - 1;

    if (start <= end)
    {
        // Normal deletion of characters from start to end
        int length = strLen - (end - start - 1);
        retval->mString = new char[length + 1];
        // Copy from the start of the string to the 'start' position
        strncpy(retval->mString, sourceStr, start);
        // Copy from the 'end + 1' position to the end of the string
        strcpy(retval->mString + start, sourceStr + end + 1);
        retval->mString[length] = '\0'; // Null-terminate the string
    }
    else
    {
        //FIXME: This doesn't work properly!
        // Exclusion case: create a string excluding the range from end to start
        int length = strLen - (end + 1) - (strLen - start);
        retval->mString = new char[length + 1];

        // Copy from the 'start' position to the end of the string
        strcpy(retval->mString, sourceStr + start - 3);

        // Copy from the start of the string to the 'end' position
        strncpy(retval->mString + (strLen - start), sourceStr, end);

        retval->mString[length] = '\0'; // Null-terminate the string
    }
}

void llInsertString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    int newSize = strlen(args[0].mString) + strlen(args[2].mString);
    retval->mString = new char[newSize];
    int insertPos = args[1].mInteger;

    // Bounds check
    if (insertPos < 0)
        insertPos = 0;
    else if (insertPos > strlen(args[0].mString))
        insertPos = strlen(args[0].mString);

    int stringPos = 0;
    //Copy the existing text
    strncpy(retval->mString, args[0].mString, insertPos);
    stringPos =+ insertPos;

    //Copy the new text
    strcpy(retval->mString + stringPos, args[2].mString);
    stringPos += strlen(args[2].mString);

    //Copy the remainder of text
    strcpy(retval->mString + stringPos, args[0].mString + insertPos);
}

void llToUpper(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    retval->mString = new char[strlen(args[0].mString)];
    strcpy(retval->mString, args[0].mString);
    for(int i=0, l=strlen(retval->mString); i<l; i++)
    {
        retval->mString[i] = toupper(retval->mString[i]);
    }
}

void llToLower(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    retval->mString = new char[strlen(args[0].mString)];
    strcpy(retval->mString, args[0].mString);
    for(int i=0, l=strlen(retval->mString); i<l; i++)
    {
        retval->mString[i] = tolower(retval->mString[i]);
    }
}

void llStringLength(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = strlen(args[0].mString);
}

//TODO: Test me
void llAxisAngle2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    retval->mQuat = LLQuaternion(args[1].mFP, args[0].mVec);
}

//TODO: Test me
void llRot2Axis(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    F32 angle_radians, x, y, z;
    args[0].mQuat.getAngleAxis(&angle_radians, &x, &y, &z);
    retval->mVec = LLVector3(x, y, z);
    retval->mVec.normalize();
}

//TODO: Test me
void llRot2Angle(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    F32 angle_radians, x, y, z;
    args[0].mQuat.getAngleAxis(&angle_radians, &x, &y, &z);
    retval->mFP = angle_radians;
}

//TODO: Test me
void llAcos(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = acos(args[0].mFP);
}

//TODO: Test me
void llAsin(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = asin(args[0].mFP);
}

void llAngleBetween(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;

    LLQuaternion quatA(args[0].mQuat);
    LLQuaternion quatB(args[1].mQuat);

    quatA.normalize();
    quatB.normalize();

    float dotProduct = dot(quatA, quatB);

    retval->mFP = std::acos(2 * dotProduct * dotProduct - 1);
}

//void llSubStringIndex(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llListSort(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llGetListLength(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    retval->mInteger = args[0].getListLength();
}

void llList2Integer(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mInteger = current->mInteger;
    }
}

void llList2Float(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mFP = current->mFP;
    }
}

void llList2String(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mString = new char[strlen(current->mString) + 1];
        strcpy(retval->mString, current->mString);
    }
}

void llList2Key(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_KEY;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mString = new char[strlen(current->mKey) + 1];
        strcpy(retval->mKey, current->mKey);
    }
}

void llList2Vector(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_VECTOR;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mVec = current->mVec;
    }
}

void llList2Rot(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_QUATERNION;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mQuat = current->mQuat;
    }
}


//void llList2List(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llDeleteSubList(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llGetListEntryType(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    int listSize = args[0].getListLength();
    int pos = args[1].mInteger;
    if (pos < 0)
        pos = listSize + pos;

    if (pos >= 0 && pos < listSize)
    {
        LLScriptLibData *current = &args[0];
        while (pos-- >= 0)
            current = current->mListp;

        retval->mInteger = current->mType;
    }
}

//void llList2CSV(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llCSV2List(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llListRandomize(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llList2ListStrided(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llListInsertList(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llListFindList(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llGetDate(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;
    time_t current_time = time_corrected();
    struct tm* time_info = gmtime(&current_time);

    retval->mString = new char[11];
    strftime(retval->mString, 11, "%Y-%m-%d", time_info);
}

//void llParseString2List(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llDumpList2String(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llMD5String(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;

    char nonce_str[16]; // Could get away with 11, but 16 should be enough for now
    std::sprintf(nonce_str, "%d", args[1].mInteger);

    // Calculate length of the resulting string
    size_t src_len = std::strlen(args[0].mString);
    size_t nonce_len = std::strlen(nonce_str);
    size_t total_len = src_len + nonce_len + 1;

    char* combined_str = new char[total_len + 1];
    std::strcpy(combined_str, args[0].mString);
    std::strcat(combined_str, ":");
    std::strcat(combined_str, nonce_str);

    LLMD5 hash(reinterpret_cast<const unsigned char*>(combined_str));
    delete[] combined_str;

    retval->mString = new char[33];
    hash.hex_digest(retval->mString);
}

//void llStringToBase64(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llBase64ToString(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llXorBase64Strings(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llLog10(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = log10(args[0].mFP);
}

void llLog(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    retval->mFP = log(args[0].mFP);
}

void llGetTimestamp(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::gmtime(&now_time_t);
    long int micros = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count() % 1000000;
    char timestamp_str[29];
    std::strftime(timestamp_str, sizeof(timestamp_str), "%Y-%m-%dT%H:%M:%S", now_tm);
    std::sprintf(timestamp_str + 19, ".%06ldZ", micros);
    retval->mString = new char[std::strlen(timestamp_str) + 1];
    std::strcpy(retval->mString, timestamp_str);
}

//void llIntegerToBase64(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llBase64ToInteger(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llGetGMTclock(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_FLOATINGPOINT;
    const time_t current_time = time_corrected();
    struct tm* time_info = gmtime(&current_time);

    retval->mFP = time_info->tm_hour * 3600 + time_info->tm_min * 60 + time_info->tm_sec;
}

//void llParseStringKeepNulls(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llListReplaceList(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llModPow(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;

    int base = args[0].mInteger;
    int exponent = args[1].mInteger;
    int modulus = args[2].mInteger;

    std::uint64_t result = 1; // Initialize result as 1, using uint64_t to handle large numbers

    // Compute base^exponent % modulus using iterative exponentiation by squaring
    while (exponent > 0)
    {
        // If exponent is odd, multiply base with result
        if (exponent % 2 == 1)
            result = (result * base) % modulus;

        // Now exponent must be even
        exponent = exponent >> 1; // Divide exponent by 2
        base = (base * base) % modulus; // Change base to base^2
    }

    retval->mInteger = static_cast<int>(result); // Store the result as an integer
}

void llEscapeURL(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id) {
    retval->mType = LST_STRING;
    size_t input_length = std::strlen(args->mString);
    retval->mString = new char[input_length * 3 + 1]; // Allocate enough space for worst-case scenario
    std::string escaped = LLURI::escape(args->mString);
    std::strcpy(retval->mString, escaped.c_str());
}

void llUnescapeURL(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id) {
    retval->mType = LST_STRING;
    size_t input_length = std::strlen(args->mString);
    retval->mString = new char[input_length + 1]; // Allocate enough space for unescaped string
    std::string unescaped = LLURI::unescape(args->mString);
    std::strcpy(retval->mString, unescaped.c_str());
}

//void llListStatistics(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llGetUnixTime(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_INTEGER;
    const time_t current_time = time_corrected();
    retval->mInteger = static_cast<int>(current_time);
}

//void llXorBase64StringsCorrect(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
//void llStringTrim(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)

void llSHA1String(LLScriptLibData *retval, LLScriptLibData *args, const LLUUID &id)
{
    retval->mType = LST_STRING;

    size_t src_len = std::strlen(args[0].mString);
    retval->mString = new char[SHA_DIGEST_LENGTH * 2 + 1];
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(args[0].mString), src_len, hash);

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        std::sprintf(retval->mString + i * 2, "%02x", hash[i]);
}

void LSLLibrary::init()
{
    mLibrary.assignExec("llSin", llSin);
    mLibrary.assignExec("llCos", llCos);
    mLibrary.assignExec("llTan", llTan);
    mLibrary.assignExec("llAtan2", llAtan2);
    mLibrary.assignExec("llSqrt", llSqrt);
    mLibrary.assignExec("llPow", llPow);
    mLibrary.assignExec("llAbs", llAbs);
    mLibrary.assignExec("llFabs", llFabs);
    mLibrary.assignExec("llFrand", llFrand);
    mLibrary.assignExec("llFloor", llFloor);
    mLibrary.assignExec("llCeil", llCeil);
    mLibrary.assignExec("llRound", llRound);
    mLibrary.assignExec("llVecMag", llVecMag);
    mLibrary.assignExec("llVecNorm", llVecNorm);
    mLibrary.assignExec("llVecDist", llVecDist);
    mLibrary.assignExec("llRot2Euler", llRot2Euler);
    mLibrary.assignExec("llEuler2Rot", llEuler2Rot);
    mLibrary.assignExec("llAxes2Rot", llAxes2Rot);
    mLibrary.assignExec("llRot2Fwd", llRot2Fwd);
    mLibrary.assignExec("llRot2Left", llRot2Left);
    mLibrary.assignExec("llRot2Up", llRot2Up);
    mLibrary.assignExec("llRotBetween", llRotBetween);

    mLibrary.assignExec("llGetWallclock", llGetWallclock);

    mLibrary.assignExec("llGetSubString", llGetSubString);
    mLibrary.assignExec("llDeleteSubString", llDeleteSubString);
    mLibrary.assignExec("llInsertString", llInsertString);

    mLibrary.assignExec("llToUpper", llToUpper);
    mLibrary.assignExec("llToLower", llToLower);
    mLibrary.assignExec("llStringLength", llStringLength);

    mLibrary.assignExec("llAxisAngle2Rot", llAxisAngle2Rot);
    mLibrary.assignExec("llRot2Axis", llRot2Axis);
    mLibrary.assignExec("llRot2Angle", llRot2Angle);
    mLibrary.assignExec("llAcos", llAcos);
    mLibrary.assignExec("llAsin", llAsin);

    mLibrary.assignExec("llAngleBetween", llAngleBetween);
    //mLibrary.assignExec("llSubStringIndex", "i", "ss");
    //mLibrary.assignExec("llListSort", "l", "lii");

    mLibrary.assignExec("llGetListLength", llGetListLength);
    mLibrary.assignExec("llList2Integer", llList2Integer);
    mLibrary.assignExec("llList2Float", llList2Float);
    mLibrary.assignExec("llList2String", llList2String);
    mLibrary.assignExec("llList2Key", llList2Key);
    mLibrary.assignExec("llList2Vector", llList2Vector);
    mLibrary.assignExec("llList2Rot", llList2Rot);

    //mLibrary.assignExec("llList2List", "l", "lii");

    //mLibrary.assignExec("llDeleteSubList", "l", "lii");
    mLibrary.assignExec("llGetListEntryType", llGetListEntryType);
    //mLibrary.assignExec("llList2CSV", "s", "l");
    //mLibrary.assignExec("llCSV2List", "l", "s");
    //mLibrary.assignExec("llListRandomize", "l", "li");
    //mLibrary.assignExec("llList2ListStrided", "l", "liii");

    //mLibrary.assignExec("llListInsertList", "l", "lli");
    //mLibrary.assignExec("llListFindList", "i", "ll");

    mLibrary.assignExec("llGetDate", llGetDate);

    //mLibrary.assignExec("llParseString2List", "l", "sll");

    //mLibrary.assignExec("llDumpList2String", "s", "ls");

    mLibrary.assignExec("llMD5String", llMD5String);

    //mLibrary.assignExec("llStringToBase64", "s", "s");
    //mLibrary.assignExec("llBase64ToString", "s", "s");
    //mLibrary.assignExec("llXorBase64Strings", "s", "ss");

    mLibrary.assignExec("llLog10", llLog10);
    mLibrary.assignExec("llLog", llLog);

    mLibrary.assignExec("llGetTimestamp", llGetTimestamp);

    //mLibrary.assignExec("llIntegerToBase64", "s", "i");
    //mLibrary.assignExec("llBase64ToInteger", "i", "s");
    mLibrary.assignExec("llGetGMTclock", llGetGMTclock);

    //mLibrary.assignExec("llParseStringKeepNulls", "l", "sll");

    //mLibrary.assignExec("llListReplaceList", "l", "llii");

    mLibrary.assignExec("llModPow", llModPow);

    mLibrary.assignExec("llEscapeURL", llEscapeURL);
    mLibrary.assignExec("llUnescapeURL", llUnescapeURL);

    //mLibrary.assignExec("llListStatistics", "f", "il");
    mLibrary.assignExec("llGetUnixTime", llGetUnixTime);

    //mLibrary.assignExec("llXorBase64StringsCorrect", "s", "ss");

    //mLibrary.assignExec("llStringTrim", "s", "si");

    mLibrary.assignExec("llSHA1String", llSHA1String);
}

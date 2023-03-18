#!/bin/bash
export
chmod +x "$(dirname "$0")/scriptUtils.sh"
source "$(dirname "$0")/scriptUtils.sh"

# ######################################################################################
# Script Functions

setupmacOS() {

    SDK_SOURCE="$SDK_PATH/redistributable_bin/osx/libsteam_api.dylib"
    assertFileHashEquals $SDK_SOURCE $SDK_HASH_OSX $ERROR_SDK_HASH
    
    echo "Copying macOS (64 bit) dependencies"
    if [[ "$YYTARGET_runtime" == "VM" ]]; then
	echo "Steamworks with VM export"
	rm "${YYoutputFolder}/libsteam_api.dylib"
        itemCopyTo "$SDK_SOURCE" "${YYoutputFolder}/libsteam_api.dylib"
	echo "Codesigning dylibs..."
	codesign -f -s "${YYPLATFORM_option_mac_signing_identity}" --timestamp --options runtime "${YYoutputFolder}/libsteam_api.dylib"
	codesign -f -s "${YYPLATFORM_option_mac_signing_identity}" --timestamp --options runtime "${YYoutputFolder}/libSteamworks.dylib"
	YOYO_VM_APP="${YYruntimeLocation}/mac/YoYo Runner.app"
	TMP_PLIST="/tmp/yysteamworks${RANDOM}.plist"
        echo "VM YoYo Runner is in: ${YOYO_VM_APP}"
	
	echo "Writing entitlements plist from ${TMP_PLIST}"
	PLIST="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n<plist version=\"1.0\">\n<dict>\n    <key>com.apple.security.cs.allow-dyld-environment-variables</key>\n    <true/>\n    <key>com.apple.security.cs.disable-library-validation</key>\n    <true/>\n</dict>\n</plist>"
	echo -e "$PLIST">"${TMP_PLIST}"
	codesign -s "${YYPLATFORM_option_mac_signing_identity}" -f --deep --timestamp --options runtime --entitlements "${TMP_PLIST}" "${YOYO_VM_APP}"
	rm "${TMP_PLIST}"
    else
        itemCopyTo "$SDK_SOURCE" "${YYprojectName}/${YYprojectName}/Supporting Files/libsteam_api.dylib"
		if [[ -z "$YYtargetFile" ]]; then
			echo "Running macOS YYC project through IDE"
			echo '[SteamworksIDE]'>>"${YYprojectName}/${YYprojectName}/Supporting Files/options.ini"
			echo 'IsRunningFromIDE=True'>>"${YYprojectName}/${YYprojectName}/Supporting Files/options.ini"
		fi
    fi
}

setupLinux() {

    SDK_SOURCE="$SDK_PATH/redistributable_bin/linux64/libsteam_api.so"
    assertFileHashEquals $SDK_SOURCE $SDK_HASH_LINUX $ERROR_SDK_HASH

    echo "Copying Linux (64 bit) dependencies"
    
    fileExtract "${YYprojectName}.zip" "_temp"
    [[ ! -f "_temp/assets/libsteam_api.so" ]] && fileCopyTo "$SDK_SOURCE" "_temp/assets/libsteam_api.so"
    if [[ -z "$YYtargetFile" ]]; then
        echo "Running Linux project through IDE"
		echo '[SteamworksIDE]'>>"_temp/assets/options.ini"
		echo 'IsRunningFromIDE=True'>>"_temp/assets/options.ini"
    fi
    folderCompress "_temp" "${YYprojectName}.zip"
    rm -r _temp
}

# ######################################################################################
# Script Logic

# Always init the script
scriptInit

# Version locks
optionGetValue "versionStable" RUNTIME_VERSION_STABLE
optionGetValue "versionBeta" RUNTIME_VERSION_BETA
optionGetValue "versionDev" RUNTIME_VERSION_DEV

# SDK Hash
optionGetValue "sdkHashWin" SDK_HASH_WIN
optionGetValue "sdkHashMac" SDK_HASH_OSX
optionGetValue "sdkHashLinux" SDK_HASH_LINUX

# SDK Path
optionGetValue "sdkPath" SDK_PATH
optionGetValue "sdkVersion" SDK_VERSION

# Debug Mode
optionGetValue "debug" DEBUG_MODE

# Error String
ERROR_SDK_HASH="Invalid Steam SDK version, sha256 hash mismatch (expected v$SDK_VERSION)."

# Checks IDE and Runtime versions
versionLockCheck "$YYruntimeVersion" $RUNTIME_VERSION_STABLE $RUNTIME_VERSION_BETA $RUNTIME_VERSION_RED

# Resolve the SDK path (must exist)
pathResolveExisting "$YYprojectDir" "$SDK_PATH" SDK_PATH

# Ensure we are on the output path
pushd "$YYoutputFolder" >/dev/null

# Call setup method depending on the platform
# NOTE: the setup method can be (:setupmacOS or :setupLinux)
setup$YYPLATFORM_name

# If debug is set to 'Enabled' provide a warning to the user.
if [ "$DEBUG_MODE" = "Enabled" ]; then
    logWarning "Debug mode is set to 'Enabled', make sure to set it to 'Auto' before publishing."
fi

popd >/dev/null

exit 0


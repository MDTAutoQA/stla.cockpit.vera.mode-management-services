#!/bin/bash

# DO NOT EDIT THIS script
# Please contribute to spx13/stla.cockpit.unity-buildscript-template
# or sync with SW Integration in case of issue
# See https://github.psa-cloud.com/spx13/stla.cockpit.unity-buildscript-template/blob/master/README.md

## uncomment to debug
#set -x

#
# Variables
set -v
MB_VERSION=2.2.8
set +v

CI="false"
CLEAN="false"

STATIC_ANALYSIS="FALSE"
UNIT_TEST_EXECUATION="parasoft"
PACK_ARTIFACTS="FALSE"
BUILD_MODULE="FALSE"

unset -v EXTRA_CMAKE_PARAMS
unset -v EXTRA_CMAKE_PARAMSX86
unset -v BUILD_ARTIFACTS
unset -v PARASOFT_INSTALL_PATH
unset -v PROJECT_NAME
unset -v MMUX_PATH
unset -v FILES_DIS_PARAINSTRUM
unset -v BUILD_NAME
unset -v YOCTOSDK_PATH
unset -v json_array
unset -v json_objects
unset -v BUILDSCRIPT_VARIABLES
unset -v KEPLER_VERSION
unset -v KEPLER_SDK_VERSION
unset -v BUILD_FLR
unset -v SDK_PATH
export json_array
export json_objects
declare -A BUILDSCRIPT_VARIABLES
declare -a PATHTO_DISABLEFILE

export isKeplerSdkRequire="FALSE"

INSTRUMENTATION_CTRL_PROP="com.parasoft.xtest.execution.api.cpp.options.instrument.file=false"
KEPLER_MW_BUILD="FALSE"
SCA_RULESET="builtin://Recommended Rules"
UT_RULESET="builtin://Generate Regression Base"
KEPLER_SDK_INSTALL_PATH="${HOME}/kepler"
KEPLER_SDK_VERSION="0.2.18865.0"
PARASOFT_INSTALL_PATH="${HOME}/parasoft/professional"
PROJECT_ROOT_DIR="$(readlink -f "$(dirname "${BASH_SOURCE}")")"
BUILD_DIR=${PROJECT_ROOT_DIR}/build
BUILD_X86DIR=${PROJECT_ROOT_DIR}/buildX86
DEPLOY_DIR=${PROJECT_ROOT_DIR}/stla-deploy
STATIC_ANALYSIS_OUTDIR=${PROJECT_ROOT_DIR}/stla-deploy/StaticAnalysis
UT_TEST_OUTDIR=${PROJECT_ROOT_DIR}/stla-deploy/UnitTest
PACK_DIR=${DEPLOY_DIR}/${BUILD_NAME}

export EXTRA_CMAKE_PARAMS
export EXTRA_CMAKE_PARAMSX86
export BUILD_ARTIFACTS
export FILES_DIS_PARAINSTRUM
export PARASOFT_INSTALL_PATH
export PROJECT_NAME
export BUILD_NAME
export MMUX_PATH
export YOCTOSDK_PATH
export KEPLER_VERSION
export KEPLER_SDK_VERSION
export BUILD_FLR
export SDK_PATH

#
# Functions
#
function logerror() {
	echo -e "\n$(date +%H:%m:%S) ERROR: $1 - exit code: $2"
	exit $2
}

function logbegin() {
	echo -e "\n$(date +%H:%m:%S) Entering $1"
}

function logend() {
	echo -e "\n$(date +%H:%m:%S) Exiting $1"
}

function fn_init_versions() {
	logbegin "fn_init_versions"
	json_array=$(perl -0777 -ne 'if (/"buildscript_variables":(.*)/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)
	json_objects=$(echo "$json_array" | sed 's/\[//;s/\]//' | tr -d '"{},')

	for object in $json_objects; do
		key=$(echo "$object" | cut -d':' -f1)
		value="$(echo "$object" | cut -d':' -f2)"
		if [ ! -z "$STLADBG" ]; then
			echo "object is <$object>"
			echo "key is <$key>"
			echo -e "value is <$value>\n"
		fi
		BUILDSCRIPT_VARIABLES[$key]=$value
	done

	# Replace ASCII HEX code with space and colon characters, so we avoid issue parsing json
	for key in "${!BUILDSCRIPT_VARIABLES[@]}"; do
		temp="$(echo ${BUILDSCRIPT_VARIABLES[$key]} | sed 's/\%3A/:/g' | sed 's/\%20/ /g')"
		echo "Using::$key with::${temp}"
		export $key="$temp"
	done
	logend "fn_init_versions"
}

#Below function parses data from config.json
#EXTRA_CMAKE_PARAMS,EXTRA_CMAKE_PARAMSX86::extra cmake params for aarch64 and x86_64 builds
#FILES_DIS_PARAINSTRUM::source file name to disable para instrum.

function fn_parse_configdata() {
	logbegin "fn_parse_configdata"
	BUILD_FLR=$(perl -ne 'if (/"build_flavor":"(.*)"/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)
	if [ "${BUILD_FLR}" == "kepler_mw" ]; then
		KEPLER_SDK_PATH=$(find "${KEPLER_SDK_INSTALL_PATH}/${KEPLER_VERSION}" -type d -name "${KEPLER_SDK_VERSION}" -print)
		if [ ! -e $KEPLER_SDK_PATH ]; then
			logerror "KEPLER_SDK_PATH, $KEPLER_SDK_PATH not existing" 1
		else
			echo "kepler SDK path ${KEPLER_SDK_PATH}"
			#source ${KEPLER_SDK_PATH}/environment-setup-sdk.sh || logerror "kepler env setupt failed" $RET
		fi
	fi

	EXTRA_CMAKE_PARAMS=$(echo "$(perl -ne 'if (/"extra_cmake_params":"(.*)"/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)" | sed -e "s|YOCTOSDK_PATH|$YOCTOSDK_PATH|g;s|KEPLER_SDK_PATH|$KEPLER_SDK_PATH|g")
	EXTRA_CMAKE_PARAMSX86=$(echo "$(perl -ne 'if (/"extra_cmake_paramsX86":"(.*)"/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)" | sed -e "s|YOCTOSDK_X86PATH|$YOCTOSDK_X86PATH|g;s|KEPLER_SDK_PATH|$KEPLER_SDK_PATH|g")
	BUILD_ARTIFACTS=$(perl -ne 'if (/"build_artifacts":\[(.*?)\]/s) { print $1 . "\n" }' config.json)
	PROJECT_NAME=$(perl -ne 'if (/"static_analysis_project_name":"(.*)"/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)
	FILES_DIS_PARAINSTRUM=$(perl -0777 -ne 'if (/"disable_parasoftinstrumention_files":\[(.*?)\]/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)
	FILES_DIS_SCA=$(perl -0777 -ne 'if (/"disable_sca_files":\s*\[\s*"([^"]*)"\s*\]/s) { print $1 . "\n" }' ${PROJECT_ROOT_DIR}/config.json)

	logend "fn_parse_configdata"
}

function fn_copy_artifacts() {
	logbegin "fn_copy_artifacts"
	echo "Packing Started"
	IFS=" " read -r -a artifacts_array <<< "$(echo "$BUILD_ARTIFACTS" | sed 's/,/ /g' | tr -d '"')"
	PACK_DIR=${DEPLOY_DIR}/${BUILD_NAME}

	[[ -d ${PACK_DIR} ]] || {
		mkdir -p ${PACK_DIR}
	}

	if [ -d "${BUILD_DIR}" ]; then

		for buildop in "${artifacts_array[@]}"; do
			cp -rvf ${BUILD_DIR}/$buildop ${PACK_DIR} || logerror "cp command failed" 9
        	done
	fi

	cd ${DEPLOY_DIR}
	if [ -d ${STATIC_ANALYSIS_OUTDIR} ]; then
		cp -rvf ${STATIC_ANALYSIS_OUTDIR} ${PACK_DIR}
	fi
	if [ -d ${UT_TEST_OUTDIR} ]; then
		cp -rvf ${UT_TEST_OUTDIR} ${PACK_DIR}
	fi

	zip -r ${BUILD_NAME}.zip ${BUILD_NAME} || logerror "zip command failed" 8

	logend "fn_copy_artifacts"
}
function fn_check_filetodisable() {
	echo "checking files to disable for instumentation  $FILES_DIS_PARAINSTRUM"
	if [ ! -z "$FILES_DIS_PARAINSTRUM" ]; then
		IFS=" " read -r -a PATHTO_DISABLEFILE <<< "$(echo "$FILES_DIS_PARAINSTRUM" | sed 's/,/ /g' | tr -d '"')"
		for f in "${PATHTO_DISABLEFILE[@]}"; do
			echo "Finding File.....${f} in ${PROJECT_ROOT_DIR}"
			path=($(find * -name $f))
			if [ ! -z $path ]; then
				for p in ${path[@]}; do
					sed -i '1 a '/${PROJECT_NAME}_Analysis/${p}//${INSTRUMENTATION_CTRL_PROP}'' .parasoft
				done
			else
				echo "$f not found in project..."
			fi
		done
	else
		echo "No file to disable instrumention"
	fi

}

function fn_import_buildx86_project() {
	logbegin "fn_import_buildx86_project"
	#MMUX_PATH=${YOCTOSDK_X86PATH}/sysroots/core2-64-poky-linux/opt/MMUX
	echo "echo ${BUILD_X86DIR}"

	if [ "${BUILD_FLR}" == "kepler_mw" ]; then
		echo "Preparing Build contents for KeplerMW apps"
		source ${KEPLER_SDK_PATH}/environment-setup-sdk.sh || logerror "kepler env setupt failed" 5
		cp -r ${KEPLER_SDK_PATH}/kndk/toolchains/x86_64/usr/include/openssl ${KEPLER_SDK_PATH}/kndk/platforms/0.2/x86_64/usr/include/openssl
		SDK_PATH=${KEPLER_SDK_PATH}/kndk/toolchains/x86_64

	else
		echo "Preparing Build contents for MW apps"
		source ${YOCTOSDK_X86PATH}/environment-setup-core2-64-poky-linux
		MMUX_PATH=${YOCTOSDK_X86PATH}/sysroots/core2-64-poky-linux/opt/MMUX
		SDK_PATH=${YOCTOSDK_X86PATH}/sysroots/x86_64-pokysdk-linux

	fi

	cmake ${EXTRA_CMAKE_PARAMSX86} -S ${PROJECT_ROOT_DIR} -B ${BUILD_X86DIR} || logerror "cmake build failed" 7

	${PARASOFT_INSTALL_PATH}/cpptest/bin/cpptesttrace --cpptesttraceTraceCommand='/x86_64-poky-linux-gcc$|/x86_64-poky-linux-g\+\+$|/gcc$|/g\+\+$' --cpptesttraceProjectName=${PROJECT_NAME}_Analysis --cpptesttraceOutputFile=$(pwd)/${PROJECT_NAME}_Analysis.bdf cmake --build ${BUILD_X86DIR} -j 64

	${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli -settings cpptest.settings -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC -property bdf.import.linker.exec=${SDK_PATH}/usr/bin/x86_64-poky-linux/x86_64-poky-linux-g++ -property -property bdf.import.cpp.compiler.exec=${SDK_PATH}/usr/bin/x86_64-poky-linux/x86_64-poky-linux-g++ -property bdf.import.compiler.family=gcc_11-64 -showdetails -appconsole stdout

	fn_check_filetodisable

	logend "fn_import_buildx86_project"

}

function fn_import_buildaarch64_project() {
	logbegin "fn_import_buildaarch64_project"
	if [ "${BUILD_FLR}" == "kepler_mw" ]; then
		echo "Preparing Build contents for MW apps Static analysis"
		source ${KEPLER_SDK_PATH}/environment-setup-sdk.sh || logerror "kepler env setupt failed" 5
		cp -r ${KEPLER_SDK_PATH}/kndk/toolchains/aarch64/usr/include/openssl ${KEPLER_SDK_PATH}/kndk/platforms/0.2/aarch64/usr/include/openssl
		SDK_PATH=${KEPLER_SDK_PATH}/kndk/toolchains/aarch64

	else
		echo "Preparing Build contents for MW apps Static analysis"
		source ${YOCTOSDK_PATH}/environment-setup-cortexa78c-poky-linux
		MMUX_PATH=${YOCTOSDK_PATH}/sysroots/cortexa78c-poky-linux/opt/MMUX
		SDK_PATH=${YOCTOSDK_PATH}/sysroots/x86_64-pokysdk-linux

	fi

	echo "echo ${BUILD_DIR}"
	rm -rf ${BUILD_DIR}
	cmake ${EXTRA_CMAKE_PARAMS} -S ${PROJECT_ROOT_DIR} -B ${BUILD_DIR} || logerror "cmake build failed" 7
	${PARASOFT_INSTALL_PATH}/cpptest/bin/cpptesttrace --cpptesttraceTraceCommand='/aarch64-poky-linux-gcc$|/aarch64-poky-linux-g\+\+$|/gcc$|/g\+\+$' --cpptesttraceProjectName=${PROJECT_NAME}_Analysis --cpptesttraceOutputFile=$(pwd)/${PROJECT_NAME}_Analysis.bdf cmake --build ${BUILD_DIR}
	#${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli  -settings cpptest.settings  -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC  -showdetails -appconsole stdout
	#${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli  -settings cpptest.settings -property bdf.import.linker.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ld -property bdf.import.cpp.compiler.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++ -property bdf.import.compiler.family=gcc_11-64 -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC  -showdetails -appconsole stdout

	logend "fn_import_buildaarch64_project"
}

function fn_run_unittest() {
	logbegin "fn_run_unittest"

	if [ "parasoft" == "$UNIT_TEST_EXECUATION" ]; then
		UT_TEST_OUTDIR="${PROJECT_ROOT_DIR}/stla-deploy/UnitTest/$UNIT_TEST_EXECUATION"

	elif [ "unity" == "$UNIT_TEST_EXECUATION" ]; then
		UT_TEST_OUTDIR="${PROJECT_ROOT_DIR}/stla-deploy/UnitTest/$UNIT_TEST_EXECUATION"

	elif [ "true" == "$UT_OVERRITE_OUTDIR" ]; then
		echo "UT_TEST_OUTDIR overriten: ${UT_TEST_OUTDIR}"

	else
		echo test
		logerror "you are using the --ut argument with the wrong value. it should be unity or parasoft"
	fi

	if [ -e ${PROJECT_ROOT_DIR}/${PROJECT_NAME}_Analysis.bdf ] && [ -d ${BUILD_X86DIR} ]; then
		echo "Project is already imported"

	else
		echo "Import and build project"
		fn_import_buildx86_project
	fi

	echo $PATH | grep -q "/sysroots/x86_64-pokysdk-linux/lib"
	if [ $? == 0 ]; then
		echo "PATH variable is already set to include SDKx86->Linker"
	else
		if [ "${BUILD_FLR}" == "kepler_mw" ]; then
			echo "setting up PATH to include SDKx86->Linker"
			export PATH=$PATH:${KEPLER_SDK_PATH}/kndk/platforms/0.2/x86_64/lib/
		else
			export PATH=$PATH:${YOCTOSDK_X86PATH}/sysroots/x86_64-pokysdk-linux/lib
		fi
	fi

	echo "Genrating Stubs"
	[[ -d ${PROJECT_ROOT_DIR}/stla-deploy ]] || {
		mkdir -m 777 -p ${UT_TEST_OUTDIR}

	}

	${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli -settings cpptest.settings -config "builtin://Generate Stubs" -showdetails -appconsole stdout

	echo "Running and execuating Unit testCases"
	${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli -settings cpptest.settings -config "${PARASOFT_INSTALL_PATH}/cpptest/configs/${UT_RULESET}" -report ${UT_TEST_OUTDIR}/ -showdetails -appconsole stdout -resource "./${PROJECT_NAME}_Analysis/tests"
	echo "${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli  -settings cpptest.settings -config "${PARASOFT_INSTALL_PATH}/cpptest/configs/${UT_RULESET}" -report ${UT_TEST_OUTDIR}/ -showdetails -appconsole stdout -resource ./${PROJECT_NAME}_Analysis"

	#need to define report name
	report_path=$(find ${UT_TEST_OUTDIR} -iname "report.xml" | grep .)
	report_found=$?

	if [ 0 -ne $report_found ]; then
		logerror "the ut report is not found" 4
	else
		grep -q 'failed="[1-9][0-9]*"' ${UT_TEST_OUTDIR}/report.xml
		if [ $? -eq 0 ]; then
			echo "UT failed on edit UT, printing xml"
			cat ${UT_TEST_OUTDIR}/report.xml

			# We always fail. In any case TC report parser will always raise error if failed UT are found

			logerror "\nUT is mandatory: there is at least one UT failed on edit UTs" 44

		else
			echo "UT ran successfully. No failed cases found."
		fi
	fi

	logend "fn_run_unittest"
}

function fn_build_keplerapp() {
	logbegin "fn_build_keplerapp"
	source ${KEPLER_SDK_PATH}/environment-setup-sdk.sh || logerror "kepler env setup failed" 5
	cp -r ${KEPLER_SDK_PATH}/kndk/toolchains/aarch64/usr/include/openssl ${KEPLER_SDK_PATH}/kndk/platforms/0.2/aarch64/usr/include/openssl
	cmake ${EXTRA_CMAKE_PARAMS} -S ${PROJECT_ROOT_DIR} -B ${BUILD_DIR} || logerror "cmake build failed" 7
	cmake --build ${BUILD_DIR} || logerror "cmake build failed" 7

	logend "fn_build_keplerapp"
}

function fn_build_project() {
	logbegin "fn_build_project"
	#fn_sourcesdk
	source ${YOCTOSDK_PATH}/environment-setup-cortexa78c-poky-linux || logerror "yoctosdk setup failed" 6
	MMUX_PATH=${YOCTOSDK_PATH}/sysroots/cortexa78c-poky-linux/opt/MMUX

	[[ -d $BUILD_DIR ]] || {
		mkdir -p build
	}

	cmake ${EXTRA_CMAKE_PARAMS} -S ${PROJECT_ROOT_DIR} -B ${BUILD_DIR}
	cmake --build ${BUILD_DIR} || logerror "cmake build failed" 7
	logend "fn_build_project"
}

#Function will source X86 SDK , build artifactes in buildx86 folder creates bdf file which is require for SCA and UT
#${PROJECT_NAME}_staticAnalysis.bdf : This file will be genrated by reffrencing  Cmake from the buildx86 folder and has all details about variable,machine,arch which is required by Parasoft cli tool

function fn_run_staticanalysis() {
	logbegin "fn_run_staticanalysis"
	[[ -d ${PROJECT_ROOT_DIR}/stla-deploy ]] || {
		mkdir -m 777 -p ${STATIC_ANALYSIS_OUTDIR}

	}
	fn_import_buildaarch64_project
	echo "Running Static analysis on Project"
	if [ -z "${FILES_DIS_SCA}" ]; then
		${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli -settings cpptest.settings -property bdf.import.linker.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ld -property bdf.import.cpp.compiler.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++ -property bdf.import.compiler.family=gcc_11-64 -config "${PARASOFT_INSTALL_PATH}/cpptest/configs/${SCA_RULESET}" -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC -report ${STATIC_ANALYSIS_OUTDIR} -showdetails -appconsole stdout -exclude '**/CMake*'
	else
		echo "excluding ${FILES_DIS_SCA}"
		${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli -settings cpptest.settings -property bdf.import.linker.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-ld -property bdf.import.cpp.compiler.exec=${SDK_PATH}/usr/bin/aarch64-poky-linux/aarch64-poky-linux-g++ -property bdf.import.compiler.family=gcc_11-64 -config "${PARASOFT_INSTALL_PATH}/cpptest/configs/${SCA_RULESET}" -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC -report ${STATIC_ANALYSIS_OUTDIR} -showdetails -appconsole stdout -exclude '**/CMake*' ${FILES_DIS_SCA}
	fi

	echo "${PARASOFT_INSTALL_PATH}/cpptest/cpptestcli  -settings cpptest.settings  -config "${PARASOFT_INSTALL_PATH}/cpptest/configs/${SCA_RULESET}" -bdf ${PROJECT_NAME}_Analysis.bdf -property bdf.import.location=BDF_LOC -report ${STATIC_ANALYSIS_OUTDIR} -showdetails -appconsole stdout -exclude '**/CMake*' ${FILES_DIS_SCA} "

	#need to define report name
	sa_report_path=$(find ${STATIC_ANALYSIS_OUTDIR} -iname "report.xml" | grep .)
	sa_report_found=$?

	if [ 0 -ne $sa_report_found ]; then
		logerror "the report for static analysis is not found" 5
	fi

	logend "fn_run_staticanalysis"
}

fn_init_versions

longopt="kepler-version:,kepler-mw-build,kepler-sdk-install-path:,kepler-sdk-version:,yoctosdk-path:,yoctosdk-x86path:,parasoft-install-path:,static-analysis-project-name:,static-analysis-outdir:,ut-outdir:,ruleset-ut:,ruleset:,build-name:,static-analysis:,ut:,pack-all,clean,help"
shortopt="kb:y:x:p:n:o:U:a:uhc"
# Call getopt to validate the provided input.
options=$(getopt -l "$longopt" -o "$shortopt" -a -- "$@")

eval set -- "$options"
while true; do
	case "$1" in
	-c | --clean)
		CLEAN="true"
		shift
		;;
	-k | --kepler-sdk-install-path)
		KEPLER_SDK_INSTALL_PATH=$2
		echo "Overriding KEPLER SDK path is ${KEPLER_SDK_INSTALL_PATH}"
		shift 2
		;;
	--kepler-sdk-version)
		KEPLER_SDK_VERSION="$2"
		echo "Overriding KEPLER SDK version is ${KEPLER_SDK_VERSION}"
		shift 2
		;;
	--kepler-version)
		KEPLER_VERSION="$2"
		echo "Overriding KEPLER version is ${KEPLER_VERSION}"
		shift 2
		;;
	--kepler-mw-build)
		KEPLER_MW_BUILD="TRUE"
		echo "Running Kepler MW build"
		shift
		;;
	-y | --yoctosdk-path)
		YOCTOSDK_PATH="$2"
		BUILD_MODULE="TRUE"
		echo "sourcing yocto sdk ${YOCTOSDK_PATH}"
		shift 2
		;;
	-x | --yoctosdk-x86path)
		YOCTOSDK_X86PATH="$2"
		echo "sourcing yocto sdk ${YOCTOSDK_PATH}"
		shift 2
		;;
	-a | --static-analysis)
		STATIC_ANALYSIS="$2"
		echo "Static analysis enabled for ${STATIC_ANALYSIS}"
		shift 2
		;;
	--ut)
		UNIT_TEST_EXECUATION="$2"
		echo "Performing UT ${UNIT_TEST_EXECUATION}"
		shift 2
		;;
	--ruleset)
		SCA_RULESET="$2"
		echo "Rulset config $SCA_RULESET"
		shift 2
		;;
	--ruleset-ut)
		UT_RULESET="$2"
		echo "Ruleset for Unittest $UT_RULESET"
		shift 2
		;;
	-n | --static-analysis-project-name)
		PROJECT_NAME="$2"
		echo "Project name is ${PROJECT_NAME}"
		shift 2
		;;
	-o | --static-analysis-outdir)
		STATIC_ANALYSIS_OUTDIR="$2"
		echo "Outdir for static analysis is ${STATIC_ANALYSIS_OUTDIR}"
		shift 2
		;;
	-U | --ut-outdir)
		UT_TEST_OUTDIR="$2"
		echo "Outdir for Unit test report is ${UT_TEST_OUTDIR}"
		shift 2
		;;
	-p | --parasoft-install-path)
		PARASOFT_INSTALL_PATH="$2"
		echo "Overriding parasoft tools for static analysis path, it is now ${PARASOFT_INSTALL_PATH}"
		shift 2
		;;
	-b | --build-name)
		BUILD_NAME="$2"
		shift 2
		;;

	--pack-all)
		echo "***PACKING***"
		PACK_ARTIFACTS="TRUE"
		shift
		;;

	-h | --help)
		SCRIPT_NAME=$(basename $0)
		echo -e "Basic usage: just invoke script with default settings\n\t$SCRIPT_NAME"
		echo -e "Options:\
				\n\t--ci: run clean and check for produced vpkg\
				\n\n-c|--clean: clean folder running git clean-dxf
				\n\t-k|--kepler-sdk-install-path: basic fodler where all kepler SDK versions are installed\
				\n\t--kepler-version: version (subfolder of install path), example 4.1.2  \
				\n\t--kepler-sdk-version: version (subfolder of install path), example 0.\
				\n\t-s|--yoctosdk-path : path where yocto sdk is installed \
				"
		echo -e "Invoke with differnt tool install path: just invoke script with default settings\n\t$SCRIPT_NAME --yoctosdk-path FOLDER "

		exit 0
		;;
	--)
		shift
		break
		;;
		#usage
	esac
done

fn_parse_configdata

echo "extra cmake ${EXTRA_CMAKE_PARAMS}"
echo "extra cmakex86 ${EXTRA_CMAKE_PARAMSX86}"

if [ "true" == "$CLEAN" ]; then
	echo -e "\nCleaning build dir\n"
	rm -rf build
	echo -e "Clean done\n"
fi

if [ $STATIC_ANALYSIS == "parasoft-linux" ]; then
	fn_run_staticanalysis

elif [ ${KEPLER_MW_BUILD} == "TRUE" ] && [ $PACK_ARTIFACTS == "FALSE" ]; then
	fn_build_keplerapp

elif [ $UNIT_TEST_EXECUATION == "parasoft" ] && [ $BUILD_MODULE == "FALSE" ] && [ ${PACK_ARTIFACTS} == "FALSE" ]; then
	fn_run_unittest

elif [ $BUILD_MODULE == "TRUE" ]; then
	echo "Building Project for TARGET ARCH64"
	fn_build_project
else
	echo "Nothing to build"

fi

if [ ${PACK_ARTIFACTS} == "TRUE" ]; then
	fn_copy_artifacts
fi

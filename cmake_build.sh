#!/bin/bash


# This batch file must be in same folder as CMakeLists.txt


# *************************
#    CONFIGURE AND BUILD
# *************************
cd "$(dirname "$0")"


# set to 0 for Debug
# set to 1 for Release
RELEASE_BUILD=1

if [ $RELEASE_BUILD -eq 0 ]
then
	echo "DEBUG BUILD"
	build_dir=build/debug/obj
	bin_dir=build/debug/bin
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S. -B$build_dir -DCMAKE_BUILD_TYPE=Debug -GNinja
else
	echo "RELEASE BUILD"
	build_dir=build/release/obj
	bin_dir=build/release/bin
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S. -B$build_dir -DCMAKE_BUILD_TYPE=Release -GNinja
fi

# Copy the generated export commands to the Build directory so that syntax handling
# can work correctly within the text editor.
echo "Copy 'compile_commands.json' to build directory"
mv $build_dir/compile_commands.json build/compile_commands.json

if [ $? -eq 0 ]
then
	cmake --build $build_dir -j4
else
	echo "Build failed" >&2
    exit 1
fi


# *************************
#     RUN APPLICATION
# *************************
if [ $? -eq 0 ]
then
	if [ -z "$1" ] 
	then
	    echo ""
	else
		#copy the executable to the bin directory
		app_name_obj=$build_dir/$1
		app_name_bin=$bin_dir/$1
		echo "Copy executable: " $app_name_obj $app_name_bin
		mv $app_name_obj $app_name_bin
		echo "Run application: " $app_name_bin
		$app_name_bin
	fi
fi


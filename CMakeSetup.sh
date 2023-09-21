if [ $# -gt 0 ]
then
    if [ $1 = "setup" ]
    then
        if [ $# -gt 1 ]
        then
            if [ -d build-$2 ]
            then
                cd build-$2
                cmake --build .. && echo "Project built successfully"
            else
                mkdir build-$2 && cd build-$2
                (cmake -DCMAKE_BUILD_TYPE=$2 .. && cmake --build . && echo "Project setup & built successfully") || 
                (cd .. && rm -rf build-$2 && echo "Project failed to setup/build")
            fi
        else
            echo "missing build configuration Debug/Release"
        fi
    elif [ $1 = "remove" ]
    then
        rm -rf build-Debug
        rm -rf build-Release
        rm -rf bin-Debug
        rm -rf bin-Release
        echo "Project removed successfully"
    elif [ $1 = "debug" ]
    then
        if [ -d bin-Debug ]
        then
            gdb bin-Debug/LC3_VM
        else
            echo "No binary available to debug"
        fi
    else
        echo "wrong argument passed" 
    fi 
else
    echo "no arguments passed !"
fi
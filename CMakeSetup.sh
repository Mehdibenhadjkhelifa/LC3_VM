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
    elif [ $1 = "run" ]
    then
        if [ $# -gt 1 ]
        then
            if [ $2 = "rogue" ]
            then
                ./bin-Release/LC3_VM ./tests/rogue.obj ||
                echo "build Release to run program more optimally"
            elif [ $2 = "2048" ]
            then
                ./bin-Release/LC3_VM ./tests/2048.obj ||
                echo "build Release to run program more optimally"
            else
                echo "Not a viable second argument,enter 2048/rogue as second arg"
            fi
        else
            echo "select 2048 or rogue as second argument to run the desired program"
        fi
    else
        echo "wrong argument passed" 
    fi 
else
    echo "no arguments passed !"
fi
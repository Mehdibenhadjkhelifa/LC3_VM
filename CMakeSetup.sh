if [ $# -gt 0 ]
then
    if [ $1 = "setup" ]
    then
        if [ -d build ]
        then
            cd build
            else
            mkdir build && cd build
            fi
        (cmake .. && cmake --build . && echo "Project setup & built successfully") || 
        (cd .. && rm -rf build && echo "Project failed to setup/build")
    elif [ $1 = "remove" ]
    then
        rm -rf build
        rm -rf bin-c
        rm -rf bin-cpp
        echo "Project removed successfully"
    else
    echo "wrong argument passed" 
    fi 
else
echo "no arguments passed !"
fi
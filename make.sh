PZL_DIR="`dirname ${0}`/fuzzle/puzzle"
PZL_PATH="`realpath ${PZL_DIR}`"
FZL_DIR="`dirname ${0}`"
FZL_PATH="`realpath ${FZL_DIR}`"
FZL_BUILD_PATH="${FZL_PATH}/fuzzle/pypzl/build"

build() 
{
    mkdir "${FZL_BUILD_PATH}"
    cd "${FZL_BUILD_PATH}"
    cmake "${PZL_PATH}"
    make
}

install()
{
    cd ${FZL_PATH}
    pip3 install . --upgrade
}

default() 
{
    build
    install
}

case $1 in
    # Default
    "" ) default ;;
    # Build
    "build" ) build ;;
esac

PZL_DIR="`dirname ${0}`/fuzzle/puzzle/"
PZL_PATH="`realpath ${PZL_DIR}`"
FZL_DIR="`dirname ${0}`"
FZL_PATH="`realpath ${FZL_DIR}`"
PZL_BUILD_PATH="${FZL_PATH}/fuzzle/puzzle/build/"
PZL_INSTALL_PATH="${HOME}/.fuzzle/"

build()
{
    # Compile puzzle
    mkdir "${PZL_BUILD_PATH}" &> /dev/null
    cd "${PZL_BUILD_PATH}"
    cmake "${PZL_PATH}" -DCMAKE_INSTALL_PREFIX=${PZL_INSTALL_PATH}
    make
}

install()
{
    cd "${PZL_BUILD_PATH}"
    make install
    cd ${FZL_PATH}
    pip3 install . --upgrade
}

default()
{
    build
    install
}

clean()
{
    rm -r "${PZL_BUILD_PATH}"
    rm -r "${PZL_INSTALL_PATH}"
    pip uninstall fuzzle -y
}

case $1 in
    # Default
    "" ) default ;;
    # Build
    "build" ) build ;;
    # Clean
    "clean" ) clean ;;
esac

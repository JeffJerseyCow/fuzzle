#!/usr/bin/env bash
FZL_DIR="`dirname ${0}`"
FZL_PATH="`realpath ${FZL_DIR}`"
FZL_INSTALL_PATH="${HOME}/.fuzzle/"
PZL_DIR="`dirname ${0}`/fuzzle/puzzle/"
PZL_PATH="`realpath ${PZL_DIR}`"
PZL_BUILD_PATH="${FZL_PATH}/fuzzle/puzzle/build/"
UZL_DIR="`dirname ${0}`/fuzzle/uuzzle"
UZL_PATH=`realpath ${UZL_DIR}`
UZL_BUILD_PATH="${FZL_PATH}/fuzzle/uuzzle/build"

build_puzzle()
{
    # Compile puzzle
    mkdir "${PZL_BUILD_PATH}" &> /dev/null
    cd "${PZL_BUILD_PATH}"
    cmake "${PZL_PATH}" -DCMAKE_INSTALL_PREFIX=${FZL_INSTALL_PATH}
    make
}

build_uuzzle()
{
  # Compile uuzzle
  mkdir "${UZL_BUILD_PATH}" &> /dev/null
  cd "${UZL_BUILD_PATH}"
  cmake "${UZL_PATH}" -DCMAKE_PREFIX_PATH=${FZL_INSTALL_PATH}
  make
}

install_puzzle()
{
    cd "${PZL_BUILD_PATH}"
    make install
}

install_fuzzle()
{
  cd ${FZL_PATH}
  pip3 install . --upgrade
}

default()
{
    build_puzzle
    install_puzzle
    build_uuzzle
    install_fuzzle
}

clean()
{
    rm -r "${PZL_BUILD_PATH}"
    rm -r "${FZL_INSTALL_PATH}"
    rm -r "${UZL_BUILD_PATH}"
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

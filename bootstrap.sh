#!/bin/sh

# Submodules
# These are not actual git submodules, at least for now. They are still just
# separate, independent repositories that are cloned here.
LIBFLUID_BASE_SUBMODULE="libfluid_base"
LIBFLUID_BASE_RELEASE="v0.1"

LIBFLUID_MSG_SUBMODULE="libfluid_msg"
LIBFLUID_MSG_RELEASE="v0.1"

GIT_REPO="https://github.com/OpenNetworkingFoundation"
GIT_REPO_DEV="git@github.com:OpenNetworkingFoundation"

# Utility functions
check() {
    if test $? -ne 0; then
        echo $1
        exit
    fi 
}

init_lib() {
    SUBMODULE="$1"
    RELEASE="$2"
    FLAG="$3"
    
    echo "Initializing '$SUBMODULE'..."
    
    # Check if submodule already exists
    if test -e "$SUBMODULE"; then
        read -r -p "File or directory '$SUBMODULE' exists. Overwrite [Y/n]? " \
        OVERWRITE;
        if [ "$OVERWRITE" = "Y" ]; then
            rm -rf $SUBMODULE
        else
            echo "Keeping '$SUBMODULE' intact and not cloning repository."
            return 0
        fi
    fi
        
    # Clone
    CLONE_PREFIX="$GIT_REPO"
    if [ "$FLAG" = "dev" ]; then
        CLONE_PREFIX="$GIT_REPO_DEV"
    fi
    git clone $CLONE_PREFIX/$SUBMODULE.git
    check "Error cloning '$SUBMODULE' repository."
    cd $SUBMODULE

    # If using the development flag, keep the repository in the master branch.
    if [ "$FLAG" = "dev" ]; then
        cd ..
        return 0
    fi
    
    # Figure out the most recent patch version for the chosen minor release
    TAG=`git describe --match "$RELEASE*" --tags --abbrev=0`
    check "Tag not found for '$SUBMODULE' $RELEASE."
    
    echo "Checking out '$SUBMODULE' $RELEASE..."
    git checkout tags/$TAG
    check "Error checking out '$SUBMODULE' $TAG."
    
    # Run autogen.sh (build autoconf tools)
    ./autogen.sh
    check "Error running autogen.sh for '$SUBMODULE'."
    cd ..
}

# Main
init_lib $LIBFLUID_BASE_SUBMODULE $LIBFLUID_BASE_RELEASE $1
init_lib $LIBFLUID_MSG_SUBMODULE $LIBFLUID_MSG_RELEASE $1
echo "Bootstrapping done."

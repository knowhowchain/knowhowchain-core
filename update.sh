#!/bin/sh
#update knowhowchain-core and submodule fc

git pull
cd libraries/fc
git checkout master
git pull

#!/bin/sh

project_version=$1

sed -i "s/%PROJECT_VERSION%/$project_version/g" ./.github/doxygen/Doxyfile

project_name="$(head -n 1 README.md | sed 's/#//')"

# use '^' as sed separator to allow '/' in project name.
sed -i "s^%PROJECT_NAME%^$project_name^g" ./.github/doxygen/Doxyfile
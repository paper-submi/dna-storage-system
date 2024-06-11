#!/bin/bash

new_path=$1

sed -i -E 's|TABLE_PATH = .*|TABLE_PATH = '"$new_path"'|' data/config
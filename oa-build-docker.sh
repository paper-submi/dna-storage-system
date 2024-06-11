#!/bin/bash

cd $(dirname $0)

tag=oligoarchive-columnar:latest

exec docker build -t "$tag" -f Dockerfile .
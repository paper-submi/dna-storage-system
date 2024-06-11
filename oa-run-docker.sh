#!/bin/bash

maindir=""
container_name=oligoarchive-columnar-container

clean=false
verbose=false
arch="`uname -m`"
tag=
platform=
while test "$#" -ne 0; do
    if test "$1" = "-C" -o "$1" = "--clean"; then
        clean=true
        shift
    elif test "$1" = "-V" -o "$1" = "--verbose"; then
        verbose=true
        shift
    else
        echo "Usage: oligoarchive-run-docker [-C|--clean] [-V|--verbose]" 1>&2
        exit 1
    fi
done

tag=oligoarchive-columnar:latest


vexec () {
    if $verbose; then
        echo "$@"
    fi
    exec "$@"
}

has_container() {
    [ $( docker ps -a | grep $container_name | wc -l ) -gt 0 ]
}

remove_containers() {
    echo "Removing all existing clouds containers..."
    docker ps -a -f name=oligoarchive-columnar --format "{{.ID}}" | while read line ; do docker rm --force $line ; done
}

start_container() {
    echo "Entering container..."
    docker start $container_name
    docker exec -it $container_name /bin/bash
}

start_new_container() {
    echo "Starting a new container..."
    vexec docker run -it \
        --name $container_name \
        -v "/media/ssd":/shared-data \
        -w "/home/oligoarchive-user" \
        $tag
}

if stat --format %i / >/dev/null 2>&1; then
    statformatarg="--format"
else
    statformatarg="-f"
fi
myfileid=`stat $statformatarg %d:%i "${BASH_SOURCE[0]}" 2>/dev/null`

dir="`pwd`"
subdir=""
while test "$dir" != / -a "$dir" != ""; do
    thisfileid=`stat $statformatarg %d:%i "$dir"/oligoarchive-run-docker 2>/dev/null`
    if test -n "$thisfileid" -a "$thisfileid" = "$myfileid"; then
        maindir="$dir"
        break
    fi
    subdir="/`basename "$dir"`$subdir"
    dir="`dirname "$dir"`"
done

if test -z "$maindir" && expr "${BASH_SOURCE[0]}" : / >/dev/null 2>&1; then
    maindir="`dirname "${BASH_SOURCE[0]}"`"
    subdir=""
fi


if test -n "$maindir"; then
    existing_image="`docker ps -f status=running -f ancestor=$tag -f volume=/host_mnt"$maindir" --no-trunc --format "{{.CreatedAt}},{{.ID}}" | sort -r | head -n 1`"
    if test -n "$existing_image"; then
        created_at="`echo $existing_image | sed 's/,.*//'`"
        image="`echo $existing_image | sed 's/^.*,//'`"
        image12="`echo $image | head -c 12`"
        echo "* Using running container $image12, created $created_at" 1>&2
        echo "- To start a new container, exit then \`oligoarchive-run-docker -f\`" 1>&2
        echo "- To kill this container, exit then \`docker kill $image12\`" 1>&2
        vexec docker exec -it $image /bin/bash
    fi
fi


if $clean; then
    remove_containers && start_new_container
elif has_container; then
    start_container
else
    start_new_container
fi
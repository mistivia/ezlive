#!/bin/bash

copy_with_path() {
  if [ "$#" -ne 2 ]; then
    echo "Usage: copy_with_path_creation <source_file> <destination_base_path>"
    return 1
  fi

  local source_file="$1"
  local destination_base_path="$2"

  local source_dir=$(dirname "$source_file")

  local destination_dir="${destination_base_path}${source_dir}"

  if mkdir -p "$destination_dir"; then
    echo "Created directory: $destination_dir"
    if cp "$source_file" "$destination_dir/"; then
      echo "Copied file: $source_file to $destination_dir/"
      return 0
    else
      echo "Error: Failed to copy file $source_file to $destination_dir/"
      return 1
    fi
  else
    echo "Error: Failed to create directory $destination_dir"
    return 1
  fi
}

if [ "$#" -ne 2 ]; then
  echo "Usage: copyldd <target_exec> <dest_dir>"
  exit 1
fi

for f in $(ldd $1 | grep "=>" | awk '{print $3}') ; do
    copy_with_path $f $2
done

cp -r $2/usr/lib64 $2/

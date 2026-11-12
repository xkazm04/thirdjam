# tile.sh <out.png> <files...>   (hstack)
out=$1; shift; ins=(); for f in "$@"; do ins+=(-i "$f"); done
ffmpeg -y -loglevel error "${ins[@]}" -filter_complex "hstack=inputs=$#" "$out"

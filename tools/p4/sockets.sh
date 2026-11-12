#!/usr/bin/env bash
# tools/p4/sockets.sh <Vendor> [hand_bone] [root_bone]: the three C-0001 sockets on SK_P4_<Vendor> through the SkeletalMeshToolset (editor on 8015).
# stick_socket on the right hand, puck_socket on the root bone 115 cm ahead on the ice (as the toy figure), head_socket on head.
set -u; export MSYS_NO_PATHCONV=1; cd "$(dirname "$0")/../.."
V="$1"; HAND="${2:-hand_r}"; ROOT="${3:-root}"; U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; MESH="/Game/P4/$V/SK_P4_$V.SK_P4_$V"
for s in "stick_socket $HAND" "puck_socket $ROOT" "head_socket head"; do set -- $s
  $M call EditorToolset.SkeletalMeshToolset.AddSocket "{\"mesh\":{\"refPath\":\"$MESH\"},\"socketName\":\"$1\",\"boneName\":\"$2\"}" $U --timeout 60 | grep -v '^{"tool"' | tr -d '\n ' | cut -c1-200; echo; done
$M call EditorToolset.SkeletalMeshToolset.SetSocketTransform "{\"mesh\":{\"refPath\":\"$MESH\"},\"socketName\":\"puck_socket\",\"location\":{\"x\":0,\"y\":115,\"z\":0},\"rotation\":{\"pitch\":0,\"yaw\":0,\"roll\":0},\"scale\":{\"x\":1,\"y\":1,\"z\":1}}" $U --timeout 60 | tail -1
$M call EditorToolset.SkeletalMeshToolset.SetSocketTransform "{\"mesh\":{\"refPath\":\"$MESH\"},\"socketName\":\"head_socket\",\"location\":{\"x\":0,\"y\":0,\"z\":30},\"rotation\":{\"pitch\":0,\"yaw\":0,\"roll\":0},\"scale\":{\"x\":1,\"y\":1,\"z\":1}}" $U --timeout 60 | tail -1
$M call EditorToolset.SkeletalMeshToolset.GetSocketNames "{\"mesh\":{\"refPath\":\"$MESH\"}}" $U --timeout 60 | grep -v '^{"tool"' | tr -d '\n ' | cut -c1-300; echo
$M call EditorToolset.AssetToolset.SaveAsset "{\"assetPath\":\"/Game/P4/$V/SK_P4_$V\"}" $U --timeout 60 | tail -1

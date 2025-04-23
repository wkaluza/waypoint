set -euo pipefail

THIS_SCRIPT_DIR="$(cd "$(dirname "$0")" >/dev/null 2>&1 && pwd)"
PROJECT_ROOT_DIR="$(realpath "${THIS_SCRIPT_DIR}/..")"
DOCKER_TAG="waypoint_development:latest"
DOCKER_BUILD_CONTEXT="${PROJECT_ROOT_DIR}/infrastructure/docker/context"
DOCKERFILE_PATH="${PROJECT_ROOT_DIR}/infrastructure/docker/build.dockerfile"
WORKDIR_IN_CONTAINER="/workspace"

LOCAL_USERNAME="$(id -un)"
LOCAL_UID="$(id -u)"
LOCAL_GID="$(id -g)"

function main
{
  docker build \
    --progress plain \
    --target base \
    --tag "${DOCKER_TAG}" \
    --file "${DOCKERFILE_PATH}" \
    --build-arg DOCKER_USERNAME="${LOCAL_USERNAME}" \
    --build-arg DOCKER_UID="${LOCAL_UID}" \
    --build-arg DOCKER_GID="${LOCAL_GID}" \
    "${DOCKER_BUILD_CONTEXT}"

  docker run \
    --interactive \
    --rm \
    --tty \
    --user "${LOCAL_USERNAME}" \
    --volume "${PROJECT_ROOT_DIR}:${WORKDIR_IN_CONTAINER}" \
    --workdir "${WORKDIR_IN_CONTAINER}" \
    "${DOCKER_TAG}" \
    bash

  echo "Success: $(basename "$0")"
}

main

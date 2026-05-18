#!/bin/bash

CONTAINER_BUILDER=${CONTAINER_BUILDER:-podman}

${CONTAINER_BUILDER} build -f Containerfile -t jxtl-build
${CONTAINER_BUILDER} create --rm --name jxtl-build-extract jxtl-build
${CONTAINER_BUILDER} cp jxtl-build-extract:packages/ ./
${CONTAINER_BUILDER} rm jxtl-build-extract
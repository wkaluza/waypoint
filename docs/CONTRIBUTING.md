# Contribution guidelines

## Contents

1. [Introduction](#introduction)
2. [Reporting defects](#reporting-defects)
3. [Proposing improvements](#proposing-improvements)
4. [Submitting pull requests](#submitting-pull-requests)
5. [Development](#development)
    1. [Development on Linux](#development-on-linux)

## Introduction

We welcome contributions from the Community, whether they are defect
reports, new feature proposals, or code changes.

To avoid wasted effort, get in touch (e.g. submit an issue) before
making changes.
Sometimes a discussion is required to determine what the best approach
is, or even if a change makes sense in the first place.
In particular, changes that radically alter Waypoint's behaviour, or
which do not align with our long-term vision will not be accepted.

In closing, let us be clear that we evaluate contributions
individually on merit.
In particular, neither your sponsor status nor time served as a
Community member guarantee that a pull request will be merged or an
issue implemented.

## Reporting defects

TODO

## Proposing improvements

TODO

## Submitting pull requests

TODO

## Development

As Waypoint currently has no dependencies, this repository is all you
need for development work.

Waypoint is a CMake project.
Make sure you have CMake installed to build it.

All source files are encoded using UTF-8.

### Development on Linux

On Linux, development is best carried out in a Docker container;
see the files in `infrastructure/docker` for details of the
environment.
To start using the container, make sure you have docker installed and
enabled for `sudo`-less execution.
Run `scripts/enter_docker.bash` in a terminal.
This script will build a Docker image with all the necessary
tooling and start a terminal session inside the container.

TODO describe using the build script

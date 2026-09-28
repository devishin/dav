#!/usr/bin/env bash
# Idempotent Cloud Agent bootstrap for the dav infrastructure repo.
# The repo holds Ansible playbooks and nginx configuration, so the dev
# environment needs the Ansible toolchain plus nginx for config validation.
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

sudo apt-get update
# ansible-core is preinstalled on the base image; add the pieces the repo needs
# for linting and validating its playbooks and nginx configs. apt install is a
# no-op when these are already present, so this stays idempotent.
sudo apt-get install -y --no-install-recommends \
  nginx \
  yamllint \
  ansible-lint

echo "== Toolchain versions =="
ansible --version | head -1
ansible-lint --version | head -1
yamllint --version
nginx -v

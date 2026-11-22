# RouteX on Hugging Face Spaces (free "CPU basic", no card needed).
#
# Copy this file and README.md from this folder into a new Docker Space.
# The Space downloads the RouteX code from GitHub when it builds, so it
# always runs the latest `main`. To redeploy after merging changes:
# Space -> Settings -> "Factory rebuild".

# ---- Build stage ----------------------------------------------------------
FROM debian:bookworm-slim AS build

RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ cmake make git ca-certificates \
 && rm -rf /var/lib/apt/lists/*

ARG ROUTEX_REPO=https://github.com/AhmedMustafa-0817/RouteX.git
ARG ROUTEX_BRANCH=main

WORKDIR /src
RUN git clone --depth 1 --branch "$ROUTEX_BRANCH" "$ROUTEX_REPO" . \
 && git log -1 --format='Building RouteX commit %h: %s'

# Build everything and run all tests; a failing test stops the deploy.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)" \
 && ctest --test-dir build --output-on-failure

# ---- Runtime stage --------------------------------------------------------
FROM debian:bookworm-slim

# Spaces run the container as user ID 1000.
RUN useradd --create-home --uid 1000 user

WORKDIR /app
COPY --from=build --chown=user:user /src/build/routex_api ./routex_api
COPY --from=build --chown=user:user /src/frontend ./frontend
# Demo data. Spaces have no persistent disk, so every restart starts again
# from this file.
COPY --from=build --chown=user:user /src/data/shipments.csv ./data/shipments.csv
RUN chown user:user /app/data

USER user

# Must match app_port in README.md.
ENV PORT=8090
EXPOSE 8090

CMD ["./routex_api"]

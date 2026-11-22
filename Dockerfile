# RouteX: C++ API + website in one container, built from this checkout.
# (For Hugging Face Spaces use deploy/huggingface/Dockerfile instead.)
#
#   docker build -t routex .
#   docker run -p 8090:8090 routex      ->  http://127.0.0.1:8090
#
# Hosting platforms set the PORT environment variable; the server reads it.

# ---- Build stage ----------------------------------------------------------
FROM debian:bookworm-slim AS build

RUN apt-get update \
 && apt-get install -y --no-install-recommends g++ cmake make \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY backend ./backend
COPY tests ./tests

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j"$(nproc)" \
 && ctest --test-dir build --output-on-failure

# ---- Runtime stage --------------------------------------------------------
FROM debian:bookworm-slim

# UID 1000 matches what Hugging Face Spaces expects; harmless elsewhere.
RUN useradd --create-home --uid 1000 routex

WORKDIR /app
COPY --from=build /src/build/routex_api ./routex_api
COPY frontend ./frontend
# Demo data. Free hosting has no persistent disk, so every restart or
# redeploy starts again from this file.
COPY --chown=routex:routex data/shipments.csv ./data/shipments.csv
RUN chown routex:routex /app/data

USER routex

ENV PORT=8090
EXPOSE 8090

CMD ["./routex_api"]

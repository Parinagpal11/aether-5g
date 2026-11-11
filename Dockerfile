FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends g++ cmake ninja-build python3 ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B /build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build /build -j 2 && ctest --test-dir /build --output-on-failure
ENTRYPOINT ["/build/aether_slots"]
CMD ["rr", "10", "100", "50", "500"]

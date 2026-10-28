# Use a lightweight Linux image with C++ tools
FROM gcc:latest

# Set the working directory
WORKDIR /app

# Copy your source code and header into the container
COPY main.cpp .
COPY httplib.h .

# Compile the C++ code (Static linking prevents missing library errors)
RUN g++ -std=c++17 main.cpp -o server -lpthread -static-libstdc++ -static-libgcc

# Expose the port the server runs on
EXPOSE 8080

# Run the server
CMD ["./server"]
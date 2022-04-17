idf_component_register(
    SRCS
        "src/model.cc"
        "src/NeuralNetwork.cpp"
    PRIV_REQUIRES tflite-lib
    INCLUDE_DIRS "src")
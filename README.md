# HDLib-C: A Hyperdimensional Computing Library in C

HDLib-C is a C library for building Vector Symbolic Architectures (VSAs) based on the principles of Hyperdimensional Computing (HDC). It is a C implementation inspired by the original Python library hdlib developed by Fabio Cumbo. This library enables the creation and manipulation of high-dimensional vectors for representing and processing information, facilitating applications in machine learning, cognitive science, natural language processing, bioinformatics, and more.

## Table of Contents
- Getting Started
- Installation
- Dependencies
- Credits
- Support and Contributions
- License

## Getting Started

### Installation
To use HDLib-C in your projects, you can clone the repository and include the necessary header and source files in your C project.

```bash
git clone https://github.com/joe-burris/hdlib-c.git
```
Include the headers in your source files:

### Dependencies
- C compiler (e.g., GCC)
- Standard C libraries
- Math library (`-lm` flag during compilation)

## Vector Symbolic Architectures

### Hyperdimensional Vectors and Space
HDLib-C provides structures and functions to create and manipulate high-dimensional vectors, which are central to VSAs. These vectors can represent symbols, data points, and other entities in a high-dimensional space, enabling complex computations and representations.

Key features include:
- Creation of random binary or bipolar vectors of specified dimensionality.
- Operations for binding, bundling, and permuting vectors.
- A space structure to manage and store vectors.

### Arithmetic Operations
The library implements essential arithmetic operations for hyperdimensional computing:
- **Bind**: Combines two vectors to produce a new vector dissimilar to the inputs.
- **Bundle**: Aggregates vectors to produce a new vector similar to the inputs.
- **Permute**: Rotates a vector's elements, used for encoding sequences or positional information.
- **Subtraction**: Computes the difference between two vectors.

These operations enable the construction of complex representations and support algorithms in machine learning and data processing.

## Credits
HDLib-C is a C implementation inspired by the Python library hdlib developed by Fabio Cumbo. We acknowledge his significant contributions to the field of hyperdimensional computing and his work on the original hdlib library.

If you use HDLib-C in your research or projects, please consider citing the original work:
> Cumbo et al., (2023). hdlib: A Python library for designing Vector-Symbolic Architectures. Journal of Open Source Software, 8(89), 5704, https://doi.org/10.21105/joss.05704

## Support and Contributions
We welcome contributions to HDLib-C! If you encounter any issues, have suggestions, or want to contribute code, please open an issue or pull request on our GitHub repository.

### How to Contribute
- **Reporting Bugs**: If you find a bug, please open an issue with detailed information on how to reproduce it.
- **Feature Requests**: If you have ideas for new features or improvements, feel free to suggest them.
- **Pull Requests**: Contributions are welcome. Please ensure your code follows the project's coding standards and includes necessary documentation.

## License
HDLib-C is released under the MIT License. This means you are free to use, modify, and distribute the code, provided you include the original license.

*Disclaimer*: This library is an independent implementation and is not affiliated with or endorsed by Fabio Cumbo. It is intended for educational and research purposes, aiming to bring the capabilities of the hdlib Python library to the C programming language while acknowledging the original author's work.


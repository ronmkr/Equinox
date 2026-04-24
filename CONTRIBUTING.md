# Contributing to Equinox

Thank you for your interest in contributing to Equinox! We welcome contributions from the community to help make this macOS audiophile equalizer even better.

## How to Contribute

### 1. Reporting Bugs
- Search existing issues to see if the bug has already been reported.
- If not, open a new issue using the **Bug Report** template.
- Provide as much detail as possible, including your macOS version, Equinox version, and steps to reproduce.

### 2. Suggesting Enhancements
- Check existing issues for similar feature requests.
- Open a new issue using the **Feature Request** template.
- Explain the "why" and "how" of your suggestion.

### 3. Code Contributions
- Fork the repository.
- Create a new branch for your feature or fix.
- Ensure your code adheres to the project's coding standards (see below).
- Run existing tests and add new ones for your changes.
- Submit a Pull Request (PR) with a clear description of your changes.

## Development Standards

### Real-Time Safety
Equinox is a real-time audio application. All DSP code must be real-time safe:
- **No memory allocation** in the audio thread.
- **No blocking calls** (I/O, mutexes).
- **Use atomics** for parameter synchronization.

### Coding Style
- We use JUCE coding standards (CamelCase for classes and methods).
- Private members should be prefixed with `m_` (e.g., `m_sampleRate`).
- Use `.clang-format` provided in the root directory.

### Pull Request Process
1. Update the `README.md` with details of changes to the interface, if applicable.
2. The PR will be merged once it has been reviewed and passes all CI checks.

## Questions?
If you have any questions, feel free to open an issue or reach out to the maintainers.

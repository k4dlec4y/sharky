#include <iostream>
#include <string>

#include "../include/bitmap.h"
#include "../include/hide.h"

std::string text =
    "In the realm of programming, clarity and maintainability are paramount. While it may be tempting to write clever or overly compact code, "
    "doing so often leads to confusion and difficulty in future maintenance. Code is read more often than it is written, and future developers "
    "—including one’s future self—will benefit immensely from code that is clear, modular, and well-documented. Readable code not only reduces "
    "errors but also facilitates collaboration, debugging, and extension of software systems.\n"
    "Good programming practices emphasize separation of concerns, consistent naming conventions, and modular design. Each function should have "
    "a single, clearly defined responsibility. Each module or class should encapsulate its own state and behavior. By adhering to these principles, "
    "developers ensure that code remains flexible and testable, allowing new features to be added without introducing regressions or instability.\n"
    "Documentation complements well-written code. Even when code is highly readable, documenting the rationale behind complex decisions or algorithms "
    "provides invaluable context. This ensures that future maintainers do not waste time deciphering the purpose or mechanics of obscure logic, and it "
    "promotes a culture of knowledge sharing and maintainability. Comments, design notes, and usage examples all contribute to a healthier codebase.\n"
    "Error handling and defensive programming are equally crucial. Anticipating possible failure modes, validating inputs, and handling exceptions "
    "gracefully ensures that software behaves predictably under a wide range of conditions. Robust error handling reduces unexpected crashes, enhances "
    "user trust, and facilitates debugging by providing informative messages and safe failure modes.\n"
    "Testing forms the backbone of reliable software. Unit tests verify the correctness of individual components, while integration tests ensure that "
    "modules interact as expected. Automated testing pipelines catch regressions early and encourage refactoring by providing a safety net. Continuous "
    "integration systems combined with thorough test coverage support iterative development and long-term maintainability.\n"
    "Version control systems, such as Git, are indispensable tools for modern software development. They allow multiple developers to work concurrently, "
    "track changes, and manage code history efficiently. Proper branching strategies, meaningful commit messages, and code reviews foster collaboration, "
    "knowledge sharing, and accountability across teams.\n"
    "Code readability also extends to formatting and style. Consistent indentation, spacing, and line length improve visual clarity, while naming conventions "
    "convey meaning without requiring extensive comments. Linters and style guides help maintain consistency across large projects, reducing cognitive load "
    "and improving maintainability.\n"
    "Refactoring is a continual process of improving code without changing its external behavior. By simplifying logic, removing duplication, and enhancing "
    "modularity, developers keep the codebase clean and adaptable. Refactoring reduces technical debt, facilitates onboarding of new team members, and "
    "prevents software rot over time.\n"
    "Performance optimization should be undertaken judiciously. Premature optimization can introduce unnecessary complexity and obscure the intent of code. "
    "Profiling identifies actual bottlenecks, allowing developers to focus on critical paths. Clear, maintainable code should generally take priority, with "
    "optimization applied where empirical evidence justifies it.\n"
    "Security considerations must be integrated from the beginning. Validating inputs, avoiding unsafe functions, and following best practices for authentication, "
    "authorization, and data handling protect software against vulnerabilities. Security-aware development reduces the likelihood of exploits and enhances user trust.\n"
    "Collaboration and code review processes improve software quality. Peer reviews catch errors that automated tests may miss, promote knowledge transfer, "
    "and foster adherence to best practices. Constructive feedback in reviews encourages growth and accountability, ultimately benefiting the project.\n"
    "Continuous learning is essential for developers. Programming languages, frameworks, and tools evolve rapidly. Keeping skills up to date ensures that "
    "developers can leverage modern features, write more efficient code, and adopt best practices. Reading books, following tutorials, contributing to open-source, "
    "and experimenting with new technologies all enhance one’s abilities.\n"
    "Software design patterns provide reusable solutions to common problems. Applying patterns judiciously helps structure code for flexibility, extensibility, "
    "and maintainability. Awareness of patterns, coupled with critical thinking about their applicability, empowers developers to create robust architectures.\n"
    "In large projects, modularity and decoupling prevent cascading failures and simplify testing. Components with well-defined interfaces allow independent development "
    "and maintenance. Dependency management, inversion of control, and interface segregation contribute to loosely coupled, highly cohesive systems.\n"
    "Logging and monitoring are essential for maintaining production systems. Capturing runtime behavior, errors, and performance metrics helps diagnose issues, "
    "analyze trends, and improve system reliability. Thoughtful logging practices strike a balance between informativeness and verbosity, preserving performance.\n"
    "User experience is an often overlooked aspect of software development. Clear, responsive, and intuitive interfaces reduce errors, increase satisfaction, "
    "and facilitate adoption. Developers should consider usability principles, accessibility standards, and consistency across the application.\n"
    "Finally, cultivating empathy for future maintainers shapes better code. Writing with readability, simplicity, and clarity in mind ensures that the next "
    "developer can understand, maintain, and extend the system efficiently. Code is ultimately a form of communication, and maintaining that communication "
    "clearly is one of the most valuable skills a programmer can possess.\n";

int main()
{
    bmp::image im = bmp::read_bmp("bitmaps/image.bmp");
    std::cout << im.width << std::endl;
    std::cout << im.height << std::endl;
    std::cout << im.byte_capacity << std::endl;
    std::cout << im.padding << std::endl;
    std::cout << im.img_data.size() << std::endl;

    std::size_t data_index = 0;
    hide_data(im, text, data_index, 136, 0);

    return bmp::write_bmp(im) ? 0 : -1;
}

SyntaxForge – Mini Expression Compiler with Intermediate Code Generation & Optimization
Overview

SyntaxForge is a mini expression compiler developed in C++ that demonstrates key concepts of Compiler Design through a practical
implementation. The project processes arithmetic assignment expressions and performs lexical analysis, symbol table management,
syntax validation, and serves as a foundation for future intermediate code generation and optimization modules. The primary
goal of SyntaxForge is to provide a transparent view of compiler internals, allowing users to observe how source code is
transformed into structured representations during compilation.


Features:

Implemented Features:
Deterministic lexical analysis
Token classification
Identifiers
Numbers
Operators
Assignment operators
Delimiters
Parentheses
Symbol table generation using STL Set
Duplicate identifier elimination
Syntax validation
Missing semicolon detection
Invalid character detection
Formatted token stream display
Formatted symbol table display

Planned Features:
Abstract Syntax Tree (AST) generation
Operator precedence parsing
Three-Address Code (TAC) generation
Constant Folding optimization
Peephole optimization
Enhanced semantic validation
Problem Statement

Many students study compiler phases theoretically but rarely observe the internal workflow of a compiler. Modern compilers
such as GCC and LLVM abstract away tokenization, symbol table generation, parsing, and intermediate code generation.

SyntaxForge bridges this gap by providing a simplified yet functional compiler framework that demonstrates how expressions
are processed through multiple compiler stages.

Objectives:
Implement lexical analysis for arithmetic expressions
Generate and maintain a symbol table
Validate statement syntax
Detect lexical and syntax errors
Generate intermediate code representations
Apply compile-time optimizations
Demonstrate compiler front-end and middle-end concepts
Technology Stack
Component	Technology
Programming Language	C++
Compiler	g++
IDE	CLion / VS Code
Data Structures	STL Vector, STL Set
Platform	Windows / Linux


Compiler Concepts Used:
Lexical Analysis
Scans the input character by character and converts lexemes into tokens.

Symbol Table Management
Stores unique identifiers encountered during compilation.

Syntax Analysis
Verifies that statements follow the required grammar rules.

Semantic Validation
Ensures valid assignment structures and expression formats.

Intermediate Code Generation
Converts expressions into machine-independent representations.

Code Optimization
Reduces redundant computations through compile-time optimizations.

Project Workflow:
Input Expression
        │
        ▼
Lexical Scanner
        │
        ▼
Token Stream Generation
        │
        ▼
Symbol Table Creation
        │
        ▼
Syntax Validation
        │
        ▼
AST Construction (Planned)
        │
        ▼
TAC Generation (Planned)
        │
        ▼
Constant Folding (Planned)
        │
        ▼
Final Output

Example Input:
a = b + c * d;
Generated Token Stream
a    IDENTIFIER
=    ASSIGNMENT
b    IDENTIFIER
+    OPERATOR
c    IDENTIFIER
*    OPERATOR
d    IDENTIFIER
;    DELIMITER
Symbol Table
a
b
c
d
Syntax Validation
VALID STATEMENT
Example Error Detection
Input
a = b + c
Output
MISSING SEMICOLON
Future Three-Address Code Output:
Input
a = b + c * d;
TAC
t1 = c * d
t2 = b + t1
a = t2

Project Structure:
SyntaxForge/
│
├── compiler.cpp
├── README.md
├── docs/
│   ├── Project_Report.pdf
│   └── Presentation.pptx
│
└── test_cases/
    ├── valid_inputs.txt
    └── invalid_inputs.txt

Building the Project:
Compile:
g++ compiler.cpp -o syntaxforge
Run:
./syntaxforge

or on Windows:
syntaxforge.exe

Sample Run
Enter Expression: a = b + c * d;

TOKEN STREAM
-------------------------
a   IDENTIFIER
=   ASSIGNMENT
b   IDENTIFIER
+   OPERATOR
c   IDENTIFIER
*   OPERATOR
d   IDENTIFIER
;   DELIMITER

SYMBOL TABLE
-------------------------
a
b
c
d

SYNTAX VALIDATION
-------------------------
VALID STATEMENT


Author:
Shubho Das
Registration Number: 24BCE2276
B.Tech Computer Science and Engineering
VIT Vellore
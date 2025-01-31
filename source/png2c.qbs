/*_____________________________________________________________________________
 |                                                                            |
 | COPYRIGHT (C) 2022 Mihai Baneu                                             |
 |                                                                            |
 | Permission is hereby  granted,  free of charge,  to any person obtaining a |
 | copy of this software and associated documentation files (the "Software"), |
 | to deal in the Software without restriction,  including without limitation |
 | the rights to  use, copy, modify, merge, publish, distribute,  sublicense, |
 | and/or sell copies  of  the Software, and to permit  persons to  whom  the |
 | Software is furnished to do so, subject to the following conditions:       |
 |                                                                            |
 | The above  copyright notice  and this permission notice  shall be included |
 | in all copies or substantial portions of the Software.                     |
 |                                                                            |
 | THE SOFTWARE IS PROVIDED  "AS IS",  WITHOUT WARRANTY OF ANY KIND,  EXPRESS |
 | OR   IMPLIED,   INCLUDING   BUT   NOT   LIMITED   TO   THE  WARRANTIES  OF |
 | MERCHANTABILITY,  FITNESS FOR  A  PARTICULAR  PURPOSE AND NONINFRINGEMENT. |
 | IN NO  EVENT SHALL  THE AUTHORS  OR  COPYRIGHT  HOLDERS  BE LIABLE FOR ANY |
 | CLAIM, DAMAGES OR OTHER LIABILITY,  WHETHER IN AN ACTION OF CONTRACT, TORT |
 | OR OTHERWISE, ARISING FROM,  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR  |
 | THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                 |
 |____________________________________________________________________________|
 |                                                                            |
 |  Author: Mihai Baneu                           Last modified: 02.Oct.2022  |
 |                                                                            |
 |___________________________________________________________________________*/

import qbs

Product {
    name: "png2c"
	//targetName: name + (qbs.buildVariant == "debug" ? "d" : "")
    type: "application"
    consoleApplication: true

    // dependencies
    Depends { name: "cpp" }
    Depends { name: "Qt.core" }
    
    // cpp module configuration
    cpp.includePaths: [ "." ]
    cpp.dynamicLibraries: ["png"]
    cpp.cxxLanguageVersion: "c++14"
    cpp.useCxxPrecompiledHeader: true

    // precompiled header
    Group {
        name: "pch files"
        files: ["stable.h"]
        fileTags: "cpp_pch_src"
    }

    // properties for the produced executable
    Group {
        qbs.install: true
        qbs.installDir: ""
        fileTagsFilter: product.type
    }

    files: [
        "main.cpp",
        "defines.h"
    ]
}

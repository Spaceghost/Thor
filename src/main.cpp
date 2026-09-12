#include <string.h>

#include "util/system.h"
#include "util/file.h"
#include "util/map.h"
#include "util/stream.h"

#include "parser.h"

#include "cg_llvm.h"

namespace Thor {
	extern const Filesystem STD_FILESYSTEM;
	extern const Heap       STD_HEAP;
	extern const Console    STD_CONSOLE;
	extern const Process    STD_PROCESS;
	extern const Linker     STD_LINKER;
	extern const Scheduler  STD_SCHEDULER;
	extern const Chrono     STD_CHRONO;
}

using namespace Thor;

int main(int argc, char **argv) {
	System sys {
		STD_FILESYSTEM,
		STD_HEAP,
		STD_CONSOLE,
		STD_PROCESS,
		STD_LINKER,
		STD_SCHEDULER,
		STD_CHRONO,
	};

	StringView filename { "test/ks.odin" };
	if (argc > 1) {
		filename = StringView { argv[1], strlen(argv[1]) };
	}

	auto parser = Parser::open(sys, filename);
	if (!parser) {
		return 1;
	}

	Array<AstRef<AstStmt>> stmts{sys.allocator};
	for (;;) {
		auto stmt = parser->parse_stmt(false, {}, {});
		if (!stmt) {
			break;
		}
		if (!stmts.push_back(move(stmt))) {
			break;
		}
	}

	auto& ast = parser->ast();
	StringBuilder builder{sys.allocator};
	for (auto stmt : stmts) {
		if (ast[stmt].is_stmt<AstEmptyStmt>()) {
			continue;
		}
		ast[stmt].dump(ast, builder, 0);
		builder.put('\n');
	}
	if (auto result = builder.result()) {
		sys.console.write(sys, *result);
		sys.console.write(sys, StringView { "\n" });
	}

	return 0;
}

// Copyright 2017-2018 ccls Authors
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "serializer.hh"

#include <string>

namespace ccls {
/*
The language client plugin needs to send initialization options in the
`initialize` request to the ccls language server.

If necessary, the command line option --init can be used to override
initialization options specified by the client. For example, in shell syntax:

  '--init={"index": {"comments": 2, "whitelist": ["."]}}'
*/
struct Config {
  // **Not available for configuration**
  std::string fallbackFolder;
  std::vector<std::pair<std::string, std::string>> workspaceFolders;
  // If specified, this option overrides compile_commands.json and this
  // external command will be executed with an option |projectRoot|.
  // The initialization options will be provided as stdin.
  // The stdout of the command should be the JSON compilation database.
  std::string compilationDatabaseCommand;
  // Directory containing compile_commands.json.
  std::string compilationDatabaseDirectory;

  struct Cache {
    // Cache directory for indexed files, either absolute or relative to the
    // project root.
    //
    // If empty, retainInMemory will be set to 1 and cache will be stored in
    // memory.
    std::string directory = ".ccls-cache";

    // Cache serialization format.
    //
    // "json" generates $directory/.../xxx.json files which can be pretty
    // printed with jq.
    //
    // "binary" uses a compact binary serialization format.
    // It is not schema-aware and you need to re-index whenever an internal
    // struct member has changed.
    SerializeFormat format = SerializeFormat::Binary;

    // If false, store cache files as $directory/@a@b/c.cc.blob
    //
    // If true, $directory/a/b/c.cc.blob. If cache.directory is absolute, make
    // sure different projects use different cache.directory as they would have
    // conflicting cache files for system headers.
    bool hierarchicalPath = false;

    // After this number of loads, keep a copy of file index in memory (which
    // increases memory usage). During incremental updates, the index subtracted
    // will come from the in-memory copy, instead of the on-disk file.
    //
    // The initial load or a save action is counted as one load.
    // 0: never retain; 1: retain after initial load; 2: retain after 2 loads
    // (initial load+first save)
    int retainInMemory = 2;
  } cache;

  struct ServerCap {
    struct DocumentOnTypeFormattingOptions {
      std::string firstTriggerCharacter = "}";
      std::vector<const char *> moreTriggerCharacter;
    } documentOnTypeFormattingProvider;

    // Set to false if you don't want folding ranges.
    bool foldingRangeProvider = true;

    struct Workspace {
      struct WorkspaceFolders {
        // Set to false if you don't want workspace folders.
        bool supported = true;

        bool changeNotifications = true;
      } workspaceFolders;
    } workspace;
  } capabilities;

  struct Clang {
    // Arguments matching any of these glob patterns will be excluded, e.g.
    // ["-fopenmp", "-m*", "-Wall"].
    std::vector<std::string> excludeArgs;

    // Additional arguments to pass to clang.
    std::vector<std::string> extraArgs;

    // Translate absolute paths in compile_commands.json entries, .ccls options
    // and cache files. This allows to reuse cache files built otherwhere if the
    // source paths are different.
    //
    // This is a list of colon-separated strings, e.g. ["/container:/host"]
    //
    // An entry of "clang -I /container/include /container/a.cc" will be
    // translated to "clang -I /host/include /host/a.cc". This is simple string
    // replacement, so "clang /prefix/container/a.cc" will become "clang
    // /prefix/host/a.cc".
    std::vector<std::string> pathMappings;

    // Value to use for clang -resource-dir if not specified.
    //
    // This option defaults to clang -print-resource-dir and should not be
    // specified unless you are using an esoteric configuration.
    std::string resourceDir;
  } clang;

  struct ClientCapability {
    // TextDocumentClientCapabilities.publishDiagnostics.relatedInformation
    bool diagnosticsRelatedInformation = true;
    // TextDocumentClientCapabilities.documentSymbol.hierarchicalDocumentSymbolSupport
    bool hierarchicalDocumentSymbolSupport = true;
    // TextDocumentClientCapabilities.definition.linkSupport
    bool linkSupport = true;
    // ClientCapabilities.workspace.semanticTokens.refreshSupport
    bool semanticTokensRefresh = true;

    // If false, disable snippets and complete just the identifier part.
    // TextDocumentClientCapabilities.completion.completionItem.snippetSupport
    bool snippetSupport = true;
  } client;

  struct CodeLens {
    // Enables code lens on parameter and function variables.
    bool localVariables = true;
  } codeLens;

  struct Completion {
    // 0: case-insensitive
    // 1: case-folded, i.e. insensitive if no input character is uppercase.
    // 2: case-sensitive
    int caseSensitivity = 2;

    // Some completion UI, such as Emacs' completion-at-point and company-lsp,
    // display completion item label and detail side by side.
    // This does not look right, when you see things like:
    //     "foo" "int foo()"
    //     "bar" "void bar(int i = 0)"
    // When this option is enabled, the completion item label is very detailed,
    // it shows the full signature of the candidate.
    // The detail just contains the completion item parent context.
    bool detailedLabel = true;

    // On large projects, completion can take a long time. By default if ccls
    // receives multiple completion requests while completion is still running
    // it will only service the newest request. If this is set to false then all
    // completion requests will be serviced.
    bool dropOldRequests = true;

    // Functions with default arguments, generate one more item per default
    // argument. That is, you get something like:
    //     "int foo()" "Foo"
    //     "void bar()" "Foo"
    //     "void bar(int i = 0)" "Foo"
    // Be wary, this is quickly quite verbose,
    // items can end up truncated by the UIs.
    bool duplicateOptional = true;

    // If true, filter and sort completion response. ccls filters and sorts
    // completions to try to be nicer to clients that can't handle big numbers
    // of completion candidates. This behaviour can be disabled by specifying
    // false for the option. This option is the most useful for LSP clients
    // that implement their own filtering and sorting logic.
    bool filterAndSort = true;

    struct Include {
      // Regex patterns to match include completion candidates against. They
      // receive the absolute file path.
      //
      // For example, to hide all files in a /CACHE/ folder, use ".*/CACHE/.*"
      std::vector<std::string> blacklist;

      // Maximum path length to show in completion results. Paths longer than
      // this will be elided with ".." put at the front. Set to 0 or a negative
      // number to disable eliding.
      int maxPathSize = 30;

      // Whitelist that file paths will be tested against. If a file path does
      // not end in one of these values, it will not be considered for
      // auto-completion. An example value is { ".h", ".hpp" }
      //
      // This is significantly faster than using a regex.
      std::vector<std::string> suffixWhitelist = {".h", ".hpp", ".hh", ".inc"};

      std::vector<std::string> whitelist;
    } include;

    // Maxmum number of results.
    int maxNum = 100;

    // Add placeholder text. Effective only if client.snippetSupport is true.
    //
    // false: foo($1)$0
    // true: foo(${1:int a}, ${2:int b})$0
    bool placeholder = true;
  } completion;

  struct Diagnostics {
    // Like index.{whitelist,blacklist}, don't publish diagnostics to
    // blacklisted files.
    std::vector<std::string> blacklist;

    // Time to wait before computing diagnostics for textDocument/didChange.
    //   -1: disable diagnostics on change
    //   0: immediately
    //   positive (e.g. 500): wait for 500 milliseconds. didChange requests in
    //     this period of time will only cause one computation.
    int onChange = 1000;

    // Time to wait before computing diagnostics for textDocument/didOpen.
    int onOpen = 0;

    // Time to wait before computing diagnostics for textDocument/didSave.
    int onSave = 0;

    bool spellChecking = true;

    std::vector<std::string> whitelist;
  } diagnostics;

  // Semantic highlighting
  struct Highlight {
    // Disable semantic highlighting for files larger than the size.
    int64_t largeFileSize = 2 * 1024 * 1024;

    // If non-zero, enable rainbow semantic tokens by assinging an extra modifier
    // indicating the rainbow ID to each symbol.
    int rainbow = 0;

    // Like index.{whitelist,blacklist}, don't publish semantic highlighting to
    // blacklisted files.
    std::vector<std::string> blacklist;

    std::vector<std::string> whitelist;
  } highlight;

  struct Index {
    // If a translation unit's absolute path matches any EMCAScript regex in the
    // whitelist, or does not match any regex in the blacklist, it will be
    // indexed. To only index files in the whitelist, add ".*" to the blacklist.
    // `std::regex_search(path, regex, std::regex_constants::match_any)`
    //
    // Example: `ash/.*\.cc`
    std::vector<std::string> blacklist;

    // 0: none, 1: Doxygen, 2: all comments
    // Plugin support for clients:
    // - https://github.com/emacs-lsp/lsp-ui
    // - https://github.com/autozimu/LanguageClient-neovim/issues/224
    int comments = 2;

    // If false, names of no linkage are not indexed in the background. They are
    // indexed after the files are opened.
    bool initialNoLinkage = false;

    // Use the two options to exclude files that should not be indexed in the
    // background.
    std::vector<std::string> initialBlacklist;
    std::vector<std::string> initialWhitelist;

    // If a variable initializer/macro replacement-list has fewer than this many
    // lines, include the initializer in detailed_name.
    int maxInitializerLines = 5;

    // If not 0, a file will be indexed in each tranlation unit that includes
    // it.
    int multiVersion = 0;

    // If multiVersion != 0, files that match blacklist but not whitelist will
    // still only be indexed for one version.
    std::vector<std::string> multiVersionBlacklist;
    std::vector<std::string> multiVersionWhitelist;

    struct Name {
      // Suppress inline and unnamed namespaces in identifier names.
      bool suppressUnwrittenScope = false;
    } name;

    // Allow indexing on textDocument/didChange.
    // May be too slow for big projects, so it is off by default.
    bool onChange = false;

    // If true, index parameters in declarations.
    bool parametersInDeclarations = true;

    // Number of indexer threads. If 0, 80% of cores are used.
    int threads = 0;

    // Whether to reparse a file if write times of its dependencies have
    // changed. The file will always be reparsed if its own write time changes.
    // 0: no, 1: only during initial load of project, 2: yes
    int trackDependency = 2;

    std::vector<std::string> whitelist;
  } index;

  struct Request {
    // If the document of a request has not been indexed, wait up to this many
    // milleseconds before reporting error.
    int64_t timeout = 5000;
  } request;

  struct Session {
    int maxNum = 10;
  } session;

  struct WorkspaceSymbol {
    int caseSensitivity = 1;
    // Maximum workspace search results.
    int maxNum = 1000;
    // If true, workspace search results will be dynamically rescored/reordered
    // as the search progresses. Some clients do their own ordering and assume
    // that the results stay sorted in the same order as the search progresses.
    bool sort = true;
  } workspaceSymbol;

  struct Xref {
    // Maximum number of definition/reference/... results.
    int maxNum = 2000;
  } xref;
};
REFLECT_STRUCT(Config::Cache, directory, format, hierarchicalPath, retainInMemory);
REFLECT_STRUCT(Config::ServerCap::DocumentOnTypeFormattingOptions, firstTriggerCharacter, moreTriggerCharacter);
REFLECT_STRUCT(Config::ServerCap::Workspace::WorkspaceFolders, supported, changeNotifications);
REFLECT_STRUCT(Config::ServerCap::Workspace, workspaceFolders);
REFLECT_STRUCT(Config::ServerCap, documentOnTypeFormattingProvider, foldingRangeProvider, workspace);
REFLECT_STRUCT(Config::Clang, excludeArgs, extraArgs, pathMappings, resourceDir);
REFLECT_STRUCT(Config::ClientCapability, diagnosticsRelatedInformation, hierarchicalDocumentSymbolSupport, linkSupport,
               snippetSupport);
REFLECT_STRUCT(Config::CodeLens, localVariables);
REFLECT_STRUCT(Config::Completion::Include, blacklist, maxPathSize, suffixWhitelist, whitelist);
REFLECT_STRUCT(Config::Completion, caseSensitivity, detailedLabel, dropOldRequests, duplicateOptional, filterAndSort,
               include, maxNum, placeholder);
REFLECT_STRUCT(Config::Diagnostics, blacklist, onChange, onOpen, onSave, spellChecking, whitelist)
REFLECT_STRUCT(Config::Highlight, largeFileSize, rainbow, blacklist, whitelist)
REFLECT_STRUCT(Config::Index::Name, suppressUnwrittenScope);
REFLECT_STRUCT(Config::Index, blacklist, comments, initialNoLinkage, initialBlacklist, initialWhitelist,
               maxInitializerLines, multiVersion, multiVersionBlacklist, multiVersionWhitelist, name, onChange,
               parametersInDeclarations, threads, trackDependency, whitelist);
REFLECT_STRUCT(Config::Request, timeout);
REFLECT_STRUCT(Config::Session, maxNum);
REFLECT_STRUCT(Config::WorkspaceSymbol, caseSensitivity, maxNum, sort);
REFLECT_STRUCT(Config::Xref, maxNum);
REFLECT_STRUCT(Config, compilationDatabaseCommand, compilationDatabaseDirectory, cache, capabilities, clang, client,
               codeLens, completion, diagnostics, highlight, index, request, session, workspaceSymbol, xref);

extern Config *g_config;

void doPathMapping(std::string &arg);
} // namespace ccls

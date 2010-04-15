//===--- FixItRewriter.h - Fix-It Rewriter Diagnostic Client ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a diagnostic client adaptor that performs rewrites as
// suggested by code modification hints attached to diagnostics. It
// then forwards any diagnostics to the adapted diagnostic client.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_FRONTEND_FIX_IT_REWRITER_H
#define LLVM_CLANG_FRONTEND_FIX_IT_REWRITER_H

#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Rewrite/Rewriter.h"
#include "llvm/ADT/SmallVector.h"

namespace llvm { class raw_ostream; }

namespace clang {

class SourceManager;
class FileEntry;

/// \brief Stores a source location in the form that it shows up on
/// the Clang command line, e.g., file:line:column.  A line and column of zero
/// indicates the whole file.
///
/// FIXME: Would prefer to use real SourceLocations, but I don't see a
/// good way to resolve them during parsing.
struct RequestedSourceLocation {
  const FileEntry *File;
  unsigned Line;
  unsigned Column;
};

class FixItRewriter : public DiagnosticClient {
  /// \brief The diagnostics machinery.
  Diagnostic &Diags;

  /// \brief The rewriter used to perform the various code
  /// modifications.
  Rewriter Rewrite;

  /// \brief The diagnostic client that performs the actual formatting
  /// of error messages.
  DiagnosticClient *Client;

  /// \brief The number of rewriter failures.
  unsigned NumFailures;

  /// \brief Locations at which we should perform fix-its.
  ///
  /// When empty, perform fix-it modifications everywhere.
  llvm::SmallVector<RequestedSourceLocation, 4> FixItLocations;

public:
  typedef Rewriter::buffer_iterator iterator;

  /// \brief Initialize a new fix-it rewriter.
  FixItRewriter(Diagnostic &Diags, SourceManager &SourceMgr,
                const LangOptions &LangOpts);

  /// \brief Destroy the fix-it rewriter.
  ~FixItRewriter();

  /// \brief Add a location where fix-it modifications should be
  /// performed.
  void addFixItLocation(RequestedSourceLocation Loc) {
    FixItLocations.push_back(Loc);
  }

  /// \brief Check whether there are modifications for a given file.
  bool IsModified(FileID ID) const {
    return Rewrite.getRewriteBufferFor(ID) != NULL;
  }

  // Iteration over files with changes.
  iterator buffer_begin() { return Rewrite.buffer_begin(); }
  iterator buffer_end() { return Rewrite.buffer_end(); }

  /// \brief Write a single modified source file.
  ///
  /// \returns true if there was an error, false otherwise.
  bool WriteFixedFile(FileID ID, llvm::raw_ostream &OS);

  /// \brief Write the modified source files.
  ///
  /// \returns true if there was an error, false otherwise.
  bool WriteFixedFiles();

  /// IncludeInDiagnosticCounts - This method (whose default implementation
  /// returns true) indicates whether the diagnostics handled by this
  /// DiagnosticClient should be included in the number of diagnostics
  /// reported by Diagnostic.
  virtual bool IncludeInDiagnosticCounts() const;

  /// HandleDiagnostic - Handle this diagnostic, reporting it to the user or
  /// capturing it to a log as needed.
  virtual void HandleDiagnostic(Diagnostic::Level DiagLevel,
                                const DiagnosticInfo &Info);

  /// \brief Emit a diagnostic via the adapted diagnostic client.
  void Diag(FullSourceLoc Loc, unsigned DiagID);
};

}

#endif // LLVM_CLANG_FRONTEND_FIX_IT_REWRITER_H

'

' Save all projects in the current solution

'

' This Visual Studio macro will walk through a solution

' and save each project.

'

' Usage:

'

'  o Make sure this macro is mapped to a keyboard shortcut.

'  o Open the solution to be saved and run the macro via the shortcut.

'

'

Option Strict Off

Option Explicit Off

Imports EnvDTE

Imports System

Imports System.Diagnostics

Imports System.Exception

Imports System.IO



Public Module SaveProjects

    Sub SaveProjects()

        Dim soln As Solution

        Dim projs As Projects

        Dim SolProj As String

        soln = DTE.Solution

        MsgBox("Saving solution " & soln.FullName())

        projs = soln.Projects

        For Each proj As Project In projs

            Try

                SolProj = Path.GetFileNameWithoutExtension(soln.FullName()) & Chr(92) & Path.GetFileNameWithoutExtension(proj.FullName())

                DTE.ActiveWindow.Object.GetItem(SolProj).Select(vsUISelectionType.vsUISelectionTypeSelect)

                DTE.ExecuteCommand("File.SaveSelectedItems")

            Catch ex As Exception

                SolProj = SolProj & " saved state is " & CStr(proj.Saved) & vbCrLf

                MsgBox(SolProj & ex.Message)

            End Try

        Next

        soln.SaveAs(soln.FullName())

    End Sub

End Module




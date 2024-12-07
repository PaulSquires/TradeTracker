#include "imgui.h"
#include "imgui_stdlib.h"

#include <fstream>
#include <ostream>
#include <vector>
#include <string>
#include <cstring>

#include "appstate.h"
#include "icons_material_design.h"
#include "utilities.h"
#include "text_input_popup.h"
#include "questionbox.h"
#include "journal_notes.h"

// #include <iostream>


std::string FOLDERS_BEGIN = "--tt-folders:begin";
std::string FOLDERS_END   = "--tt-folders:end";
std::string NOTE_BEGIN    = "--tt-note:begin";
std::string NOTE_END      = "--tt-note:end";
std::string NOTE_FOLDER   = "--tt-note:folder:";
std::string NOTE_TRASH    = "--tt-note:trash";


int GetFolderVectorIndex(AppState& state, int folder_id) {
    // Get the index into the journal folders vector for the selected folder id
    for (int i = 0; i < state.vector_folders.size(); ++i) {
        if (state.vector_folders.at(i).id == folder_id) {
            return i;
        }
    }
    return -1;
}


int GetNoteVectorIndex(AppState& state, int note_id) {
    // Get the index into the journal notes vector for the selected note id
    for (int i = 0; i < state.vector_notes.size(); ++i) {
        if (state.vector_notes.at(i).id == note_id) {
            return i;
        }
    }
    return -1;
}


void UpdateFolderNotesCount(AppState& state) {
    // Start by resetting all note counts
    state.vector_folders_top_panel.at(0).notes_count = 0;  // All Notes
    state.vector_folders_top_panel.at(1).notes_count = 0;  // Trash

    for (auto& folder : state.vector_folders) {
        folder.notes_count = 0;
    }

    // Loop through the Notes and update the correspondng folder counts
    for (const auto& note : state.vector_notes) {
        // If marked for Permanent deletion then simply bypass saving the Note.
        if (note.is_perm_delete) continue;
        if (note.folder_id == JOURNAL_NOTES_TRASH_ID) {
            state.vector_folders_top_panel.at(1).notes_count++;
        } else {
            int folder_index = GetFolderVectorIndex(state, note.folder_id);
            if (folder_index != -1) {
                state.vector_folders_top_panel.at(0).notes_count++;
                state.vector_folders.at(folder_index).notes_count++;
            }
        }
    }
}


void LoadJournalNotes(AppState& state) {
    bool reading_folders = false;
    bool reading_note = false;

    std::string note_text;
    std::string note_folder;
    bool note_trash = false;

    state.vector_folders_top_panel.clear();
    state.vector_folders.clear();
    state.vector_notes.clear();

    // Add the two always available folders (All Folders & Trash)
    JournalFolder folder;
    folder.id = JOURNAL_NOTES_ALLNOTES_ID; 
    folder.description = "All Notes";
    state.vector_folders_top_panel.push_back(folder);

    folder.id = JOURNAL_NOTES_TRASH_ID; 
    folder.description = "Trash";
    state.vector_folders_top_panel.push_back(folder);

    // The first Journal Folder is always "Notes" and is used as the
    // default location where notes are stored in cases where the user
    // has not chosen a specific folder.
    folder.id = JOURNAL_NOTES_NOTES_ID; 
    folder.description = "Notes";
    state.vector_folders.push_back(folder);

    std::string line;
    std::ifstream file(state.db.dbJournalNotes);

    while (std::getline(file, line)) {
        if (line.substr(0, FOLDERS_BEGIN.length()) == FOLDERS_BEGIN) {reading_folders = true; continue;}
        if (line.substr(0, FOLDERS_END.length()) == FOLDERS_END) {reading_folders = false; continue;}

        if (line.substr(0, NOTE_BEGIN.length()) == NOTE_BEGIN) {
            reading_note = true;
            note_text = "";
            note_folder = ""; 
            note_trash = false;
            continue;
        }

        if (line.substr(0, NOTE_END.length()) == NOTE_END) {
            JournalNote note;
            note.id = state.journal_notes_next_id++; 
            note.folder = note_folder;
            
            // Lookup the folder_id from the journal_folders vector
            note.folder_id = JOURNAL_NOTES_TRASH_ID;
            if (!note_trash && note.folder.length()) {
                for (auto& folder : state.vector_folders) {
                    if (folder.description == note.folder) {
                        note.folder_id = folder.id;
                        break;
                    }
                }
            }

            note.text = note_text;
            note.is_trash = note_trash;
            state.vector_notes.push_back(note);
            reading_note = false;
            continue;
        }

        if (reading_folders) {
            JournalFolder folder;
            folder.id = state.journal_notes_next_id++; 
            folder.description = line;
            state.vector_folders.push_back(folder);
            continue;
        }

        if (reading_note) {
            if (line.substr(0, NOTE_FOLDER.length()) == NOTE_FOLDER) {
                note_folder = line.substr(NOTE_FOLDER.length());
            } else if (line.substr(0, NOTE_TRASH.length()) == NOTE_TRASH) {
                note_trash = true;
            } else {
                note_text += (line + "\n");
            }
            continue;
        }
    }
    file.close();

    UpdateFolderNotesCount(state);

    state.is_journalnotes_data_loaded = true;
}


void SaveJournalNotes(AppState& state) {
    if (state.journalnotes_notes_modified) {
        std::ofstream file(state.db.dbJournalNotes);

        file << FOLDERS_BEGIN << '\n';
        for (auto folder : state.vector_folders) {
            if (folder.id != JOURNAL_NOTES_NOTES_ID) {
                file << folder.description << '\n';
            }
        }
        file << FOLDERS_END << "\n\n";

        for (auto note : state.vector_notes) {
            // If marked for Permanent deletion then simply bypass saving the Note.
            if (note.is_perm_delete) continue;

            file << NOTE_BEGIN << '\n';
            if (note.folder.length()) {
                file << NOTE_FOLDER << note.folder << '\n';
            }
            if (note.is_trash) {
                file << NOTE_TRASH << '\n';
            }
            // Ensure that the Note text is terminated with a linefeed otherwise
            // the closing tt tag will not start on the next line.
            if (!note.text.ends_with('\n')) note.text += '\n'; 
            file << note.text;
            file << NOTE_END << "\n\n";
        }
        file.close();

        state.journalnotes_notes_modified = false;
    }
}


std::string GetFolderDisplayName(AppState& state, int folder_id) {
    std::string folder_name;
    int notes_count = 0;

    if (folder_id == JOURNAL_NOTES_ALLNOTES_ID) {
        folder_name = state.vector_folders_top_panel.at(0).description;
        notes_count = state.vector_folders_top_panel.at(0).notes_count;
    } else if (folder_id == JOURNAL_NOTES_TRASH_ID) {
        folder_name = state.vector_folders_top_panel.at(1).description;
        notes_count = state.vector_folders_top_panel.at(1).notes_count;
    } else {
        int folder_index = GetFolderVectorIndex(state, folder_id);
        if (folder_index != -1) {
            folder_name = state.vector_folders.at(folder_index).description;
            notes_count = state.vector_folders.at(folder_index).notes_count;
        }
    }
    folder_name += " (" + std::to_string(notes_count) + ")";
    return folder_name; 
}


void RenameJournalFolder(AppState& state) {
    if (state.gettextpopup_cancelled) return;

    int folder_id = state.journal_folders_selected_id;
    int folder_index = GetFolderVectorIndex(state, folder_id);
    if (folder_index == -1) return;

    JournalFolder& folder = state.vector_folders.at(folder_index);
    
    // Need to update all Notes that ha the original folder name to the new name
    for (auto& note : state.vector_notes) {
        if (note.folder_id == folder.id) {
            note.folder = state.gettextpopup_result_string;
        }
    }

    // Set the folder's new name
    folder.description = state.gettextpopup_result_string;

    state.journalnotes_notes_modified = true;
}


void AddJournalFolder(AppState& state) {
    if (state.gettextpopup_cancelled) return;
    JournalFolder folder;
    folder.id = state.journal_notes_next_id++;
    folder.description = state.gettextpopup_result_string;
    // Add the new folder immediately after the fixed "Notes" folder
    state.vector_folders.insert(state.vector_folders.begin() + 1, folder);
    state.journalnotes_notes_modified = true;
}


void DeleteJournalFolder(AppState& state) {
    int folder_index = GetFolderVectorIndex(state, state.journal_folders_selected_id);
    if (folder_index == -1) return;

    JournalFolder& folder = state.vector_folders.at(folder_index);

    // If the Folder has Notes then tag those notes for the Trash.
    for (auto& note : state.vector_notes) {
        if (note.folder_id == folder.id) {
            note.folder = "";
            note.folder_id = JOURNAL_NOTES_TRASH_ID;
            note.is_trash = true;
        }
    }

    // Update the Folder notes counts
    UpdateFolderNotesCount(state);

    // Delete the Folder
    state.vector_folders.erase(state.vector_folders.begin() + folder_index);
    state.journalnotes_notes_modified = true;

    // Set the selected Folder back to the "All Notes" special folder
    state.current_vector_index_top_panel = 0;
    state.current_vector_index_bottom_panel = -1;
    state.journal_folders_selected_id = JOURNAL_NOTES_ALLNOTES_ID; 
    state.journal_notes_selected_id = -1;
}


void AddJournalNote(AppState& state) {
    JournalNote note;
    note.id = state.journal_notes_next_id++;
    if (state.journal_folders_selected_id == JOURNAL_NOTES_ALLNOTES_ID) {
        // The "Notes" folder is always first in the vector
        note.folder = state.vector_folders.at(0).description;
        note.folder_id = JOURNAL_NOTES_NOTES_ID;   
    } else {
        int folder_index = GetFolderVectorIndex(state, state.journal_folders_selected_id);
        JournalFolder& folder = state.vector_folders.at(folder_index);
        note.folder = folder.description;
        note.folder_id = folder.id;
    }

    note.text = "New Note\n";
    state.vector_notes.insert(state.vector_notes.begin(), note);
    state.journal_notes_selected_id = note.id;

    // Update the Folder notes counts
    UpdateFolderNotesCount(state);

    state.journalnotes_notes_modified = true;
}


void DeleteJournalNote(AppState& state, int note_id) {
    int note_index = GetNoteVectorIndex(state, note_id);
    if (note_index == -1) return;
    JournalNote& note = state.vector_notes.at(note_index);
    note.folder_id = JOURNAL_NOTES_TRASH_ID;
    note.folder = "";
    note.is_trash = true;
    state.journal_notes_selected_id = -1;
    UpdateFolderNotesCount(state);
    state.journalnotes_notes_modified = true;
}


void PermanentDeleteNote(AppState& state) {
    int note_index = GetNoteVectorIndex(state, state.journal_notes_selected_id);
    if (note_index == -1) return;
    JournalNote& note = state.vector_notes.at(note_index);
    note.is_perm_delete = true;
    state.journal_notes_selected_id = -1;
    UpdateFolderNotesCount(state);
    state.journalnotes_notes_modified = true;
}


void RestoreJournalNote(AppState& state, int note_id) {
    int note_index = GetNoteVectorIndex(state, note_id);
    if (note_index == -1) return;
    JournalNote& note = state.vector_notes.at(note_index);
    note.folder_id = JOURNAL_NOTES_NOTES_ID;
    note.folder = state.vector_folders.at(0).description;   // "Notes"
    note.is_trash = false;
    state.journal_notes_selected_id = -1;
    UpdateFolderNotesCount(state);
    state.journalnotes_notes_modified = true;
}


void MoveNoteToFolder(AppState& state, int note_id) {
    int note_index = GetNoteVectorIndex(state, note_id);
    if (note_index == -1) return;
    JournalNote& note = state.vector_notes.at(note_index);

    std::string folder_name;
    bool is_moved = false;

    // Allow Note to be moved to "Trash" only if it is not already in the trash
    if (!note.is_trash) {
        folder_name = state.vector_folders_top_panel.at(1).description;
        if (ImGui::MenuItem(folder_name.c_str())) {
            note.folder = folder_name;
            note.folder_id = JOURNAL_NOTES_TRASH_ID;
            note.is_trash = true;
            is_moved = true;
        }
    }
    for (const auto& folder : state.vector_folders) {
        // Bypass showing the Folder that the Note is currently in.
        if (folder.id == note.folder_id) continue;;
        folder_name = folder.description;
        if (ImGui::MenuItem(folder_name.c_str())) {
            note.folder = folder_name;
            note.folder_id = folder.id;
            note.is_trash = false;
            is_moved = true;
        }
    }
    ImGui::EndMenu();

    if (is_moved) {
        state.journal_notes_selected_id = -1;
        UpdateFolderNotesCount(state);
        state.journalnotes_notes_modified = true;
    }
}


void ShowJournalFoldersRightClickPopup(AppState& state) {
    if (state.show_journalfolders_rightclickmenu) {
        ImGui::OpenPopup("JournalFoldersRightClickMenuPopup");
        state.show_journalfolders_rightclickmenu = false;
    }

    if (ImGui::BeginPopup("JournalFoldersRightClickMenuPopup")) {

        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        // Change background color when the item is hot tracked
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

        if (ImGui::MenuItem("      Rename Folder")) {
            int folder_index = GetFolderVectorIndex(state, state.journal_folders_selected_id);
            if (folder_index == -1) return;
            CustomGetTextPopup(state, "Rename Folder", "Enter new folder name:", 
                state.vector_folders.at(folder_index).description, QuestionCallback::RenameJournalFolder);
        }
        if (ImGui::MenuItem("      Delete Folder")) {
            std::string text = "Are you sure you want to delete this folder? All notes in the folder will be moved to the Trash.";
            CustomQuestionBox(state, "Delete Folder", text.c_str(), QuestionCallback::DeleteJournalFolder);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("      Add Folder")) {
            CustomGetTextPopup(state, "New Folder", "Enter new folder name:", "", QuestionCallback::AddJournalFolder);
        }

        // The "Notes" folder does not produce a right click popup so we only have to deal with
        // user created folders that can be moved.
        ImGui::Separator();
        int folder_index = GetFolderVectorIndex(state, state.journal_folders_selected_id);

        if (folder_index > 1 && ImGui::MenuItem("      Move Folder Up")) {
            std::swap(state.vector_folders[folder_index], state.vector_folders[folder_index - 1]);
            state.current_vector_index_bottom_panel--;
            state.journalnotes_notes_modified = true;
        }
        if (folder_index < state.vector_folders.size() - 1 && ImGui::MenuItem("      Move Folder Down")) {
            std::swap(state.vector_folders[folder_index], state.vector_folders[folder_index + 1]);
            state.current_vector_index_bottom_panel++;
            state.journalnotes_notes_modified = true;
        }

        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
}


void ShowJournalNotesRightClickPopup(AppState& state) {
    if (state.show_journalnotes_rightclickmenu) {
        ImGui::OpenPopup("JournalNotesRightClickMenuPopup");
        state.show_journalnotes_rightclickmenu = false;
    }

    if (ImGui::BeginPopup("JournalNotesRightClickMenuPopup")) {

        ImGui::PushStyleColor(ImGuiCol_Text, clrTextLightWhite(state));
        // Change background color when the item is hot tracked
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));

        bool is_trash_folder = (state.journal_folders_selected_id == JOURNAL_NOTES_TRASH_ID);

        // If the "Trash" folder is selected then allow the Note to be Restored.
        if (is_trash_folder) {
            if (ImGui::MenuItem("      Restore Note")) {
                RestoreJournalNote(state, state.journal_notes_selected_id);   
            }
        }
        if (ImGui::MenuItem("      Delete Note")) {
            if (is_trash_folder) {
                std::string text = "Are you sure you want to delete this note permanently? It will not be recoverable.";
                CustomQuestionBox(state, "Permanently Delete Note", text.c_str(), QuestionCallback::PermanentDeleteNote);
            } else {
                DeleteJournalNote(state, state.journal_notes_selected_id);   
            }
        }

        if (ImGui::BeginMenu("      Move to")) {
            MoveNoteToFolder(state, state.journal_notes_selected_id);
        }

        // Allow adding a New Note only if the "Trash" folder is not selected
        if (!is_trash_folder) {
            ImGui::Separator();
            if (ImGui::MenuItem("      New Note")) {
                AddJournalNote(state);
            }
        }

        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
}


void DisplayNotesCount(AppState& state, int notes_count) {
    std::string notes_count_string = std::to_string(notes_count) + " ";
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    float text_width = ImGui::CalcTextSize(notes_count_string.c_str()).x;
    ImVec2 pos{max.x - text_width, min.y};
    draw_list->AddText(pos, clrTextDarkWhite(state), notes_count_string.c_str());
}


std::string AddTextWithEllipsis(std::string text, float max_width) {
    // Measure the full text size
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str());

    // If text fits within the maxWidth, just render it
    if (text_size.x <= max_width) return text;

    // Otherwise, we need to truncate and add ellipses
    std::string ellipsis = "...";
    ImVec2 ellipsis_size = ImGui::CalcTextSize(ellipsis.c_str());

    // Start truncating character by character
    int text_len = text.length();
    for (int i = text_len - 1; i > 0; --i) {
        ImVec2 truncated_text_size = ImGui::CalcTextSize(text.substr(0, i).c_str());
        if (truncated_text_size.x + ellipsis_size.x <= max_width) {
            return text.substr(0, i) + ellipsis;
        }
    }

    // In case no truncation is possible, just show ellipses
    return ellipsis;
}


float CreateDisplayNote(AppState& state, std::vector<std::string>& vec_lines, JournalNote& note, float max_width) {
    // CreateDisplayNote returns height of the text to be displayed
    // Parse the first 3 lines of the text and remove 
    // all leading spacing and line endings.
    std::istringstream stream(note.text);
    std::string token;
    int max_lines = 5;

    // Each display block is composed of 5 or 6 lines:
    //    <blank line>
    //    line 1 text
    //    line 2 text
    //    line 3 text
    //    (optional folder name)   // "All Notes" selection will have an additional line to indicate the folder
    //    <blank line>

    vec_lines.clear();

    vec_lines.push_back("\n");
    int numlines = 1;
    while (std::getline(stream, token, '\n')) {
        token = AfxTrim(token);
        if (token.length()) {
            token = AddTextWithEllipsis(token, max_width);
            vec_lines.push_back(token);
            numlines++;
        }
        if (numlines == 4) break;
    }


    // Ensure that the vector has max lines by filling remaining with line endings
    for (int i = numlines; i < max_lines; ++i) {
        vec_lines.push_back("\n");
    }

    // If the All Notes folder is selected then we also display the note's folder so that
    // the user knows from where the note belongs.
    if (state.journal_folders_selected_id == JOURNAL_NOTES_ALLNOTES_ID) {
        std::string folder_name = ICON_MD_NOTES;
        folder_name += " " + note.folder;
        vec_lines.push_back(folder_name);
    }

    return (ImGui::CalcTextSize("Thing").y * vec_lines.size());
}


void ShowJournalNotes(AppState& state) {
    if (!state.show_journalnotes) return;

    if (!state.is_journalnotes_data_loaded) {
        LoadJournalNotes(state);
    }

    // Get the available window size
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    window_size.x -= state.dpi(24);

    static float panel1_size = MAX(state.dpi(200.0f), window_size.x * 0.18f);
    static float panel2_size = MAX(state.dpi(200.0f), window_size.x * 0.18f);
    const float splitter_width = state.dpi(4.0f);
    const float min_panel_size = state.dpi(50.0f);     // Minimum size for each pane

    ImU32 back_color = clrBackDarkGray(state);
    ImU32 text_color_white = clrTextLightWhite(state);
    ImU32 text_color_gray = clrTextDarkWhite(state);

    float left = 20.0f;
    float dpi_left = state.dpi(left);

    static std::vector<std::string> vec_lines;

    // Calculate the size of the third pane
    float panel3_size = window_size.x - panel1_size - panel2_size - (2 * splitter_width);

    ImGui::PushStyleColor(ImGuiCol_Header, clrSelection(state));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, clrSelection(state));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, clrSelection(state));
    ImGui::PushStyleColor(ImGuiCol_Border, back_color);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, back_color);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, back_color);    
    ImGui::PushStyleColor(ImGuiCol_Text, text_color_white);

    // First panel
    ImGui::BeginChild("##Panel1", ImVec2(panel1_size, window_size.y), true);
    ImGui::NewLine();
    TextLabel(state, "JOURNAL NOTES", left, text_color_white, back_color);
    ImGui::NewLine();

    ImGui::SetNextItemWidth(panel1_size - (dpi_left * 2));
    ImGui::SetCursorPosX(dpi_left);
    const int ids_top_list[] = { JOURNAL_NOTES_ALLNOTES_ID, JOURNAL_NOTES_TRASH_ID };
    if (ImGui::BeginListBox("##listbox_panel1_top", ImVec2(panel1_size - (dpi_left * 2), state.dpi(75.0f)))) {
        int folder_index = -1;
        for (int i = 0; i < IM_ARRAYSIZE(ids_top_list); ++i) {
            std::string folder_name = ICON_MD_NOTES; folder_name += "  " + state.vector_folders_top_panel.at(i).description;
            const bool is_selected = (state.current_vector_index_top_panel == i);
            if (ImGui::Selectable(folder_name.c_str(), is_selected) ||
                ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                state.current_vector_index_top_panel = i;
                state.current_vector_index_bottom_panel = -1;
                state.journal_folders_selected_id = ids_top_list[i]; 
                state.journal_notes_selected_id = -1;
            }

            // Display the Notes Count
            DisplayNotesCount(state, state.vector_folders_top_panel.at(i).notes_count);
        }
        ImGui::EndListBox();
    }



    ImGui::NewLine();
    TextLabel(state, "FOLDERS", left, text_color_gray, back_color);
    ImVec2 button_size = ImVec2{state.dpi(22.0f), state.dpi(22.0f)}; 
    if (ColoredButton(state, ICON_MD_ADD, state.undpi(panel1_size) - 22.0f - left, button_size, clrTextDarkWhite(state), back_color)) {
        CustomGetTextPopup(state, "New Folder", "Enter new folder name:", "", QuestionCallback::AddJournalFolder);
    }
    Tooltip(state, "Create New Folder", clrTextLightWhite(state), clrBackMediumGray(state));

    int size = state.vector_folders.size();

    ImGui::SetCursorPosX(dpi_left);
    if (ImGui::BeginListBox("##listbox_panel1_bottom", 
            ImVec2(panel1_size - (dpi_left * 2), window_size.y - ImGui::GetCursorPosY()))) {
        for (int i = 0; i < size; i++) {
            const bool is_selected = (state.current_vector_index_bottom_panel == i);
            std::string folder_name = ICON_MD_NOTES;
            folder_name += "  " + state.vector_folders.at(i).description;
            if (ImGui::Selectable(folder_name.c_str(), is_selected) ||
                ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                state.current_vector_index_bottom_panel = i;
                state.current_vector_index_top_panel = -1;
                state.journal_folders_selected_id = state.vector_folders.at(i).id; 
                state.journal_notes_selected_id= -1;

                // Only allow a right click menu if this is not the "Notes" folder (i==0)
                if (i != 0 && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    state.show_journalfolders_rightclickmenu = true;
                }
            }

            // Display the Notes Count
            DisplayNotesCount(state, state.vector_folders.at(i).notes_count);
        }
        ImGui::EndListBox();
    }
        
    ImGui::EndChild();



    // Adjust the splitter between first and second panels
    ImGui::SameLine();
    ImGui::Button("##Splitter1", ImVec2(splitter_width, state.dpi(-1.0f)));
    if (ImGui::IsItemActive()) {
        float mouseDelta = ImGui::GetIO().MouseDelta.x;
        panel1_size += mouseDelta;
        panel1_size = MAX(MIN(panel1_size, window_size.x-min_panel_size), min_panel_size);
    }
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }


    
    // Second panel
    ImGui::SameLine();
    ImGui::BeginChild("##Panel2", ImVec2(panel2_size, window_size.y), true);

    std::string folder_name = GetFolderDisplayName(state, state.journal_folders_selected_id);
    float line_height = ImGui::CalcTextSize("Thing").y + state.dpi(2.0f);
    TextLabelCentered(state, folder_name.c_str(), 0.0f, ImVec2(panel2_size - state.dpi(48.0f), line_height), text_color_white, back_color);
    button_size = ImVec2{state.dpi(22.0f), state.dpi(22.0f)}; 
    if (ColoredButton(state, ICON_MD_ADD, state.undpi(panel2_size) - 22.0f - left, button_size, clrTextDarkWhite(state), back_color)) {
        AddJournalNote(state);
    }
    Tooltip(state, "Create New Note", clrTextLightWhite(state), clrBackMediumGray(state));
    ImGui::NewLine();

    if (ImGui::BeginListBox("##listbox_panel2", ImVec2{-1,-1})) {
        for (int i = 0; i < state.vector_notes.size(); i++) {
            JournalNote& note = state.vector_notes.at(i);

            // If marked for Permanent deletion then simply bypass the Note.
            if (note.is_perm_delete) continue;

            // Filter the Notes based on the selected folder
            bool display_note = false;
            bool is_trash = note.is_trash;

            if (state.journal_folders_selected_id == JOURNAL_NOTES_ALLNOTES_ID) {
                if (!is_trash) display_note = true;
            } else if (state.journal_folders_selected_id == JOURNAL_NOTES_TRASH_ID) {
                if (is_trash) display_note = true;
            } else {
                int folder_index = GetFolderVectorIndex(state, state.journal_folders_selected_id);
                JournalFolder& folder = state.vector_folders.at(folder_index);
                if (note.folder_id == folder.id) {
                    if (!is_trash) display_note = true;
                }
            }

            if (display_note) {
                if (state.journal_notes_selected_id == -1) state.journal_notes_selected_id = note.id;
                bool is_selected = (state.journal_notes_selected_id == note.id);

                float max_textline_width = panel2_size - state.dpi(30.0f);
                float note_display_text_height = CreateDisplayNote(state, vec_lines, note, max_textline_width);
                ImVec2 rect(panel2_size-state.dpi(12), note_display_text_height); 
                std::string id = "##" + std::to_string(i);

                if (ImGui::Selectable(id.c_str(), is_selected, 0, rect) ||
                    ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    state.journal_notes_selected_id = note.id;
                }
            
                if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                    state.show_journalnotes_rightclickmenu = true;
                }

                // Get the current window's drawing list
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Custom drawing for each item
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();

                if (is_selected) {
                    // Draw a custom background for the selected item
                    draw_list->AddRectFilled(min, max, IM_COL32(100, 149, 237, 40)); // Cornflower blue

                    // Draw a border around the item
                    draw_list->AddRect(min, max, clrBackLightGray(state));
                }

                // Draw the separator line
                ImVec2 p1{min.x + state.dpi(15), max.y};                    // Start point
                ImVec2 p2{min.x + panel2_size - state.dpi(30), max.y};      // End point
                draw_list->AddLine(p1, p2, clrBackLightGray(state), 1.5f);  // (start, end, color, thickness)

                ImU32 clr = text_color_white;
                float y_pos = min.y;
                for (int i = 0; i < vec_lines.size(); ++i) {
                    ImVec2 pos{min.x + state.dpi(15), y_pos};
                    draw_list->AddText(pos, clr, vec_lines.at(i).c_str());
                    y_pos += ImGui::CalcTextSize(vec_lines.at(i).c_str()).y;
                    if (i > 0) clr = text_color_gray;
                }
            }

        }
        ImGui::EndListBox();
    }
        
    ImGui::EndChild();


    
    // Adjust the splitter between second and third panels
    ImGui::SameLine();
    ImGui::Button("##Splitter2", ImVec2(splitter_width, state.dpi(-1.0f)));
    if (ImGui::IsItemActive()) {
        float mouseDelta = ImGui::GetIO().MouseDelta.x;
        panel2_size += mouseDelta;
        panel2_size = MAX(MIN(panel2_size, window_size.x-panel1_size-min_panel_size), min_panel_size);
        panel3_size -= mouseDelta;
        panel3_size = MAX(panel3_size, min_panel_size);
    }
    if (ImGui::IsItemActive() || ImGui::IsItemHovered()) {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    }


    
    // Third panel
    ImGui::SameLine();
    ImGui::BeginChild("##Panel3", ImVec2(panel3_size, window_size.y), true);
    static std::string journal_notes_text;
    if (state.journal_notes_selected_id >= 0) {
        int note_index = GetNoteVectorIndex(state, state.journal_notes_selected_id);
        if (note_index != -1) {
            JournalNote& note = state.vector_notes.at(note_index);
            journal_notes_text = note.text;
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(state.dpi(15), state.dpi(15)));
            bool modified = ImGui::InputTextMultiline("##JournalNotesMultiLine", &journal_notes_text, ImVec2(panel3_size, -1));
            ImGui::PopStyleVar();
            if (modified) {
                note.text = journal_notes_text;
                state.journalnotes_notes_modified = true;
            }
        }
    }
    ImGui::EndChild();


    // If textbox loses focus but text had been modified then write it to the file
    if (!ImGui::IsItemFocused()) {
        SaveJournalNotes(state);
    }

    ImGui::PopStyleColor(7);  
}
 

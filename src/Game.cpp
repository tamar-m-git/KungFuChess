#include "Game.hpp"
#include "CaptureRules.hpp"
#include <opencv2/opencv.hpp>
#include <set>
#include "Physics.hpp"

// ---------------- Implementation --------------------
Game::Game(std::vector<PiecePtr> pcs, Board board)
    : pieces(pcs), board(board) {
    validate();
    
    // Initialize event system
    audioManager_ = std::make_shared<AudioManager>();
    eventPublisher_.subscribe("piece_moved", audioManager_);
    eventPublisher_.subscribe("piece_captured", audioManager_);
    eventPublisher_.subscribe("game_started", audioManager_);
    eventPublisher_.subscribe("game_ended", audioManager_);
    
    for(const auto & p : pieces) {
        if (p) {
            piece_by_id[p->id] = p;
        }
    }
    start_tp = std::chrono::steady_clock::now();
    // Initialize position map
    update_cell2piece_map();
}

int Game::game_time_ms() const {
    return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_tp).count());
}

Board Game::clone_board() const {
    return board.clone();
}

void Game::run(int num_iterations, bool is_with_graphics) {
    // Publish game start event
    eventPublisher_.publish(GameEvent("game_started"));
    
    running_ = true;
    start_user_input_thread();
    int start_ms = game_time_ms();
    // Don't call reset - it breaks piece positions
    // for(auto & p : pieces) p->reset(start_ms);

    run_game_loop(num_iterations, is_with_graphics);

    announce_win();
    
    // Wait for user to see the victory message and hear the sound
    if(is_with_graphics) {
        std::cout << "Press any key to exit..." << std::endl;
        cv::waitKey(0); // Wait for any key press
        OpenCvImg::close_all_windows();
    } else {
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
    }
    
    running_ = false;
}

void Game::start_user_input_thread() {
    running_ = true;
}

void Game::run_game_loop(int num_iterations, bool is_with_graphics) {
    int it_counter = 0;
    // Initialize all pieces first
    int now = game_time_ms();
    for(size_t i = 0; i < pieces.size(); ++i) {
        auto& p = pieces[i];
        try {
            p->update(now);
        } catch (const std::exception& e) {
            // Silent error handling
        }
    }
    
    while(!is_win() && running_) {
        now = game_time_ms();
        
        // Update all pieces
        for(auto & p : pieces) {
            p->update(now);
        }

        update_cell2piece_map();
        
        // Check for pawn promotion after pieces update
        if (!is_promoting_) {
            for(auto & piece : pieces) {
                if (needs_promotion(piece)) {
                    handle_pawn_promotion(piece);
                    break; // Handle one promotion at a time
                }
            }
        }

        // Input processing moved to graphics section where OpenCV handles keys

        if(is_with_graphics) {
            // Create a copy of the board to draw pieces on
            auto display_board = board.clone();
            
            int pieces_drawn = 0;
            int pieces_failed = 0;
            
            // Draw all pieces on the board
            for(const auto& piece : pieces) {
                auto cell = piece->current_cell();
                
                try {
                    if (!piece->state) {
                        pieces_failed++;
                        continue;
                    }
                    
                    if (!piece->state->graphics) {
                        pieces_failed++;
                        continue;
                    }
                    
                    // Update graphics before getting image
                    piece->state->graphics->update(now);
                    auto piece_img = piece->state->graphics->get_img();
                    if (!piece_img) {
                        pieces_failed++;
                        continue;
                    }
                    


                    // Use physics position for moving pieces, cell position for static pieces
                    std::pair<int, int> pos_pix;
                    if (piece->state->name == "move" || piece->state->name == "jump") {
                        auto pos_m = piece->state->physics->get_pos_m();
                        pos_pix = piece->state->physics->get_pos_pix();
                        // Fallback: if position is (0,0), use cell position instead
                        if (pos_m.first == 0.0 && pos_m.second == 0.0) {
                            auto pos_m_fallback = display_board.cell_to_m(cell);
                            pos_pix = display_board.m_to_pix(pos_m_fallback);
                        }
                    } else {
                        auto pos_m = display_board.cell_to_m(cell);
                        pos_pix = display_board.m_to_pix(pos_m);
                    }

                    piece_img->draw_on(*display_board.img, pos_pix.first, pos_pix.second);
                    pieces_drawn++;
                    
                } catch (const std::exception& e) {
                    pieces_failed++;
                }
            }
            
            if (pieces_drawn > 0) {
                // Draw green border around current cursor position
                auto cursor_pos_m = display_board.cell_to_m(cursor_pos_);
                auto cursor_pos_pix = display_board.m_to_pix(cursor_pos_m);
                int cell_size = 80;
                display_board.img->draw_rect(cursor_pos_pix.first, cursor_pos_pix.second, 
                                           cell_size, cell_size, {0, 255, 0}); // Green border
                
                // Draw blue border around selected piece
                if (selected_piece_) {
                    auto selected_pos_m = display_board.cell_to_m(selected_piece_pos_);
                    auto selected_pos_pix = display_board.m_to_pix(selected_pos_m);
                    display_board.img->draw_rect(selected_pos_pix.first, selected_pos_pix.second, 
                                               cell_size, cell_size, {255, 0, 0}); // Blue border
                }
                
                // Show promotion message if in promotion mode
                if (is_promoting_) {
                    // Draw promotion text on screen
                    std::string promotion_text = "PROMOTION: Press Q/R/B/N";
                    // Note: OpenCV text drawing would go here in a real implementation
                }
                
                display_board.show();
                
                // Handle input in main loop where window exists
                int key = cv::waitKeyEx(30);
                if (key != -1) {
                    Command cmd(game_time_ms(), "", "", {});
                    
                    // Arrow keys controls (final: swap 8 and 2)
                    if (key == 2424832) cmd = Command(game_time_ms(), "", "up", {});    // 4 -> Up
                    else if (key == 2555904) cmd = Command(game_time_ms(), "", "down", {});  // 6 -> Down  
                    else if (key == 2490368) cmd = Command(game_time_ms(), "", "left", {});  // 8 -> Left
                    else if (key == 2621440) cmd = Command(game_time_ms(), "", "right", {}); // 2 -> Right
                    else if (key == 13) cmd = Command(game_time_ms(), "", "select", {});  // Enter
                    else if (key == 32) cmd = Command(game_time_ms(), "", "jump_action", {});  // Space
                    else if (key == 'q' || key == 'Q') cmd = Command(game_time_ms(), "", "promote_queen", {});
                    else if (key == 'r' || key == 'R') cmd = Command(game_time_ms(), "", "promote_rook", {});
                    else if (key == 'b' || key == 'B') cmd = Command(game_time_ms(), "", "promote_bishop", {});
                    else if (key == 'n' || key == 'N') cmd = Command(game_time_ms(), "", "promote_knight", {});
                    else if (key == 27) { // ESC
                        return;
                    }
                    
                    if (!cmd.type.empty()) {
                        process_input(cmd);
                    }
                }
            }
        }

        resolve_collisions();
        
        // Check win condition after resolving collisions
        if (is_win()) {
            break; // Exit game loop immediately when win condition is met
        }

        ++it_counter;
        // Run indefinitely unless ESC is pressed or win condition is met
        
        // Frame pacing handled by cv::waitKey(30)
    }
}

void Game::update_cell2piece_map() {
    std::lock_guard<std::mutex> lock(positions_mutex_);
    pos.clear();
    for(const auto& p : pieces) {
        if (p && p->state && p->state->physics) {
            auto cell = p->current_cell();
            // Validate cell coordinates
            if (cell.first >= 0 && cell.first < board.W_cells && 
                cell.second >= 0 && cell.second < board.H_cells) {
                pos[cell].push_back(p);
            }
        }
    }
}

void Game::process_input(const Command& cmd) {
    std::lock_guard<std::mutex> lock(input_mutex_);
    
    if (cmd.type == "up") move_cursor(0, -1);
    else if (cmd.type == "down") move_cursor(0, 1);
    else if (cmd.type == "left") move_cursor(-1, 0);
    else if (cmd.type == "right") move_cursor(1, 0);
    else if (cmd.type == "select") {
        // Update position map before accessing it
        update_cell2piece_map();
        
        // Enhanced selection logic from movement-logic branch
        if (selected_piece_ == nullptr) {
            // First press - pick up piece under cursor
            auto cell_pieces_it = pos.find(cursor_pos_);
            if (cell_pieces_it != pos.end() && !cell_pieces_it->second.empty()) {
                selected_piece_ = cell_pieces_it->second[0];
                selected_piece_pos_ = cursor_pos_;
            }
        } else if (cursor_pos_ == selected_piece_pos_) {
            // Same position - deselect
            selected_piece_ = nullptr;
            selected_piece_pos_ = {-1, -1};
        } else {
            // Different position - validate and create move command
            if (is_move_valid(selected_piece_, selected_piece_pos_, cursor_pos_)) {
                try {
                    Command move_cmd(cmd.timestamp, selected_piece_->id, "move", {selected_piece_pos_, cursor_pos_});
                    
                    // Process the move command through state machine
                    auto piece_it = piece_by_id.find(move_cmd.piece_id);
                    if (piece_it != piece_by_id.end()) {
                        auto piece = piece_it->second;
                        if (piece && piece->state) {
                            // Update position map again before passing to piece
                            update_cell2piece_map();
                            piece->on_command(move_cmd, pos);
                            
                            // Publish move event
                            std::unordered_map<std::string, std::string> eventData;
                            eventData["piece_id"] = selected_piece_->id;
                            eventData["from"] = std::to_string(selected_piece_pos_.first) + "," + std::to_string(selected_piece_pos_.second);
                            eventData["to"] = std::to_string(cursor_pos_.first) + "," + std::to_string(cursor_pos_.second);
                            eventPublisher_.publish(GameEvent("piece_moved", eventData));
                        }
                    }
                } catch (const std::exception& e) {
                    // Handle move command errors silently
                }
            }
            // If move is invalid, silently ignore (piece stays selected)
            
            // Reset selection
            selected_piece_ = nullptr;
            selected_piece_pos_ = {-1, -1};
        }
    }
    else if (cmd.type == "jump_action") {
        if (selected_piece_ != nullptr) {
            // Jump in place - no movement, just state change
            Command jump_cmd(cmd.timestamp, selected_piece_->id, "jump", {selected_piece_pos_});
            
            auto piece_it = piece_by_id.find(jump_cmd.piece_id);
            if (piece_it != piece_by_id.end()) {
                auto piece = piece_it->second;
                if (piece && piece->state) {
                    update_cell2piece_map();
                    piece->on_command(jump_cmd, pos);
                }
            }
            
            // Reset selection
            selected_piece_ = nullptr;
            selected_piece_pos_ = {-1, -1};
        }
    }
    else if (is_promoting_ && (cmd.type.find("promote_") == 0)) {
        if (promoting_pawn_) {
            std::string piece_type;
            if (cmd.type == "promote_queen") piece_type = "Q";
            else if (cmd.type == "promote_rook") piece_type = "R";
            else if (cmd.type == "promote_bishop") piece_type = "B";
            else if (cmd.type == "promote_knight") piece_type = "N";
            
            if (!piece_type.empty()) {
                auto position = promoting_pawn_->current_cell();
                char color = promoting_pawn_->id[1];
                
                // Create new promoted piece
                auto new_piece = create_promoted_piece(piece_type, position, color);
                
                // Replace old pawn with new piece
                auto it = std::find(pieces.begin(), pieces.end(), promoting_pawn_);
                if (it != pieces.end()) {
                    pieces.erase(it);
                }
                piece_by_id.erase(promoting_pawn_->id);
                
                // Add new piece
                pieces.push_back(new_piece);
                piece_by_id[new_piece->id] = new_piece;
                
                std::cout << "Pawn promoted to " << piece_type << "!" << std::endl;
                
                // Reset promotion state
                promoting_pawn_ = nullptr;
                is_promoting_ = false;
                
                update_cell2piece_map();
            }
        }
    }
}

void Game::move_cursor(int dx, int dy) {
    cursor_pos_.first = std::max(0, std::min(board.W_cells - 1, cursor_pos_.first + dx));
    cursor_pos_.second = std::max(0, std::min(board.H_cells - 1, cursor_pos_.second + dy));
}

void Game::resolve_collisions() {
    check_captures();
}

void Game::announce_win() const {
    // Find remaining kings to determine winner
    std::vector<PiecePtr> remaining_kings;
    for (const auto& piece : pieces) {
        if (piece->id.at(0) == 'K') {
            remaining_kings.push_back(piece);
        }
    }
    
    if (remaining_kings.size() == 1) {
        char winner_color = remaining_kings[0]->id[1];
        std::string winner_name = (winner_color == 'W') ? "WHITE" : "BLACK";
        
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "🏆 GAME OVER! 🏆" << std::endl;
        std::cout << "🎉 " << winner_name << " WINS! 🎉" << std::endl;
        std::cout << "Remaining King: " << remaining_kings[0]->id << std::endl;
        std::cout << std::string(50, '=') << "\n" << std::endl;
        
        // Publish game end event
        std::unordered_map<std::string, std::string> eventData;
        eventData["winner"] = winner_name;
        eventData["winner_color"] = std::string(1, winner_color);
        const_cast<Game*>(this)->eventPublisher_.publish(GameEvent("game_ended", eventData));
    } else if (remaining_kings.size() == 0) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "💥 DRAW! Both kings were captured! 💥" << std::endl;
        std::cout << std::string(50, '=') << "\n" << std::endl;
        
        // Publish draw event
        std::unordered_map<std::string, std::string> eventData;
        eventData["result"] = "DRAW";
        const_cast<Game*>(this)->eventPublisher_.publish(GameEvent("game_ended", eventData));
    }
}

void Game::validate() {
    if(pieces.empty()) {
        throw InvalidBoard("No pieces provided");
    }
    
    if(board.W_cells <= 0 || board.H_cells <= 0) {
        throw InvalidBoard("Invalid board dimensions");
    }
}

bool Game::is_win() const {
    int king_count = 0;

    for (const auto& piece : pieces) {
        if (piece->id.at(0)=='K') {
            king_count++;
        }
    }
    return king_count < 2;
}


void Game::enqueue_command(const Command& cmd) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    user_input_queue.push(cmd);
    cv_.notify_one();
}

void Game::handle_mouse_click(int x, int y) {
    std::lock_guard<std::mutex> lock(input_mutex_);
    if (!is_selecting_target_) {
        select_piece_at(x, y);
    } else {
        if (selected_piece_) {
            auto start_cell = selected_piece_->current_cell();
            if (is_move_valid(selected_piece_, start_cell, {x, y})) {
                Command move_cmd(game_time_ms(), selected_piece_->id, "move", {start_cell, {x, y}}, 1);
                enqueue_command(move_cmd);
            }
        }
        cancel_selection();
    }
}

void Game::handle_key_press(int key) {
    std::lock_guard<std::mutex> lock(input_mutex_);
    switch(key) {
        case 27: cancel_selection(); break;
        case 13: confirm_move(); break;
        case 2424832: move_cursor(0, -1); break;  // 4 -> Up
        case 2555904: move_cursor(0, 1); break;   // 6 -> Down
        case 2490368: move_cursor(-1, 0); break;  // 8 -> Left
        case 2621440: move_cursor(1, 0); break;   // 2 -> Right
    }
}

void Game::select_piece_at(int x, int y) {
    auto cell_pieces_it = pos.find({x, y});
    if (cell_pieces_it != pos.end() && !cell_pieces_it->second.empty()) {
        selected_piece_ = cell_pieces_it->second[0];
        cursor_pos_ = {x, y};
        is_selecting_target_ = true;
    }
}

void Game::confirm_move() {
    if (selected_piece_ && is_selecting_target_) {
        auto start_cell = selected_piece_->current_cell();
        if (is_move_valid(selected_piece_, start_cell, cursor_pos_)) {
            Command move_cmd(game_time_ms(), selected_piece_->id, "move", {start_cell, cursor_pos_}, 1);
            enqueue_command(move_cmd);
        }
    }
    cancel_selection();
}

void Game::cancel_selection() {
    selected_piece_ = nullptr;
    is_selecting_target_ = false;
}

std::string Game::cell_to_chess_notation(int x, int y) {
    return std::string(1, 'a' + y) + std::to_string(x + 1);
}

PiecePtr Game::find_piece_by_id(const std::string& id) {
    auto it = piece_by_id.find(id);
    return (it != piece_by_id.end()) ? it->second : nullptr;
}

void Game::check_captures() {
    // Create a copy of the position map to avoid iterator invalidation
    auto pos_copy = pos;
    
    static std::set<std::string> already_captured; // Keep track globally
    static std::set<std::pair<std::string, std::string>> reported_collisions; // Track reported collisions
    
    for (const auto& [cell, pieces_at_cell] : pos_copy) {
        if (pieces_at_cell.size() > 1) {
            CaptureRules::print_collision_summary(cell, pieces_at_cell);
            
            // בדוק כל זוג חתיכות
            for (size_t i = 0; i < pieces_at_cell.size(); ++i) {
                for (size_t j = i + 1; j < pieces_at_cell.size(); ++j) {
                    auto piece1 = pieces_at_cell[i];
                    auto piece2 = pieces_at_cell[j];
                    
                    // Validate pieces before processing
                    if (piece1 && piece2 && piece1->state && piece2->state && 
                        piece1->id.length() >= 2 && piece2->id.length() >= 2) {
                        
                        // Enhanced capture callback with Knight logic
                        auto capture_callback = [this, cell](PiecePtr captured, PiecePtr captor) {
                            // Skip capture if knight is not at its target destination
                            if (captor->id.size() > 0 && captor->id[0] == 'N' && 
                                captor->state->physics->end_cell != cell) {
                                return; // Knight doesn't capture unless at target
                            }
                            this->capture_piece(captured, captor);
                        };
                        
                        if (CaptureRules::process_collision_pair(piece1, piece2, already_captured, 
                                                                     reported_collisions, game_time_ms(), 
                                                                     capture_callback)) {
                            return; // Exit after first capture
                        }
                    }
                }
            }
        }
    }
}

void Game::capture_piece(PiecePtr captured, PiecePtr captor) {
    std::cout << "capture_piece CALLED: " << (captured ? captured->id : "NULL") 
              << " captured by " << (captor ? captor->id : "NULL") << std::endl;
    
    if (captured && captor) {
        // Remove the captured piece first
        pieces.erase(std::remove(pieces.begin(), pieces.end(), captured), pieces.end());
        piece_by_id.erase(captured->id);
        update_cell2piece_map();
         // אחרי update_cell2piece_map() – בודקים אם נשארו פחות משני מלכים
    if (is_win()) {
        // בונים נתוני אירוע סיום
        std::unordered_map<std::string, std::string> endData;
        if (!pieces.empty()) {
            char winner_color = pieces[0]->id[1];
            endData["winner"] = (winner_color == 'W' ? "WHITE" : "BLACK");
            endData["winner_color"] = std::string(1, winner_color);
        } else {
            endData["result"] = "DRAW";
        }
        // שולחים אירוע סיום ומשמיעים את הצליל המתאים
        eventPublisher_.publish(GameEvent("game_ended", endData));
        announce_win();
        running_ = false;
        return;  // מחזירים כדי לא לפרסם את אירוע ה‑capture הרגיל
    }

            // Normal capture - play capture sound
            std::unordered_map<std::string, std::string> eventData;
            eventData["captured"] = captured->id;
            eventData["captor"] = captor->id;
            eventPublisher_.publish(GameEvent("piece_captured", eventData));
        }
    }


std::string Game::get_position_key(int x, int y) {
    return std::to_string(x) + "," + std::to_string(y);
}

bool Game::is_move_valid(PiecePtr piece, const std::pair<int,int>& from, const std::pair<int,int>& to) {
    if (!piece || !piece->state || !piece->state->moves) {
        std::cout << "MOVE_VALIDATION: Invalid piece or state" << std::endl;
        return false;
    }
    
    std::cout << "MOVE_VALIDATION: " << piece->id << " מנסה לזוז מ-(" << from.first << "," << from.second 
              << ") ל-(" << to.first << "," << to.second << ")" << std::endl;
    
    // Check if piece can move (not in rest state)
    if (piece->state->name == "long_rest" || piece->state->name == "short_rest") {
        return false;
    }
    
    // Check if piece is currently moving
    if (piece->state->name == "move" || piece->state->name == "jump") {
        return false;
    }
    
    // Check bounds
    if (to.first < 0 || to.first >= board.H_cells || 
        to.second < 0 || to.second >= board.W_cells) {
        return false;
    }
    
    // Check if trying to move to same position
    if (from == to) {
        return false;
    }
    
    // Verify piece is actually at the 'from' position
    auto current_cell = piece->current_cell();
    if (current_cell != from) {
        return false;
    }
    
    // Update position map to ensure accuracy
    update_cell2piece_map();
    
    // Check if destination has piece of same color
    auto dest_it = pos.find(to);
    if (dest_it != pos.end() && !dest_it->second.empty()) {
        auto dest_piece = dest_it->second[0];
        if (are_same_color(piece, dest_piece)) {
            return false;
        }
    }
    
    // Create set of occupied cells for path checking
    std::unordered_set<std::pair<int,int>, PairHash> occupied_cells;
    for (const auto& [cell, pieces_at_cell] : pos) {
        if (!pieces_at_cell.empty()) {
            occupied_cells.insert(cell);
            std::cout << "OCCUPIED CELL: (" << cell.first << "," << cell.second << ") בה נמצא " << pieces_at_cell[0]->id << std::endl;
        }
    }
    
    // Check if target cell has pieces and what team they are
    auto target_pieces_it = pos.find(to);
    if (target_pieces_it != pos.end() && !target_pieces_it->second.empty()) {
        auto target_piece = target_pieces_it->second[0];
        if (target_piece && target_piece->id.length() >= 2 && piece->id.length() >= 2) {
            char moving_team = piece->id[1];
            char target_team = target_piece->id[1];
            std::cout << "MOVE_VALIDATION: " << piece->id << " (team " << moving_team 
                      << ") wants to move to cell with " << target_piece->id << " (team " << target_team << ")" << std::endl;
            
            if (moving_team == target_team) {
                std::cout << "MOVE_VALIDATION: BLOCKED - Same team!" << std::endl;
                return false; // Block same-team moves
            }
        }
    }
    
    std::cout << "CALLING piece->state->moves->is_valid() עם " << occupied_cells.size() << " תאים תפוסים" << std::endl;
    bool result = piece->state->moves->is_valid(from, to, occupied_cells);
    std::cout << "MOVE_VALIDATION: Result = " << (result ? "VALID" : "INVALID") << std::endl;
    
    if (!result) {
        std::cout << "MOVE FAILED ANALYSIS:" << std::endl;
        std::cout << "  Piece type: " << piece->id[0] << std::endl;
        std::cout << "  Distance: dx=" << abs(to.first - from.first) << ", dy=" << abs(to.second - from.second) << std::endl;
        std::cout << "  Path blocking check needed..." << std::endl;
    }
    
    return result;
}

char Game::get_piece_color(PiecePtr piece) {
    if (!piece || piece->id.size() < 2) {
        return '?';
    }
    // Color is the second character (after piece type): PW, PB, RW, etc.
    return piece->id[1]; // Second character is color (W/B)
}

bool Game::are_same_color(PiecePtr piece1, PiecePtr piece2) {
    if (!piece1 || !piece2) {
        return false;
    }
    char color1 = get_piece_color(piece1);
    char color2 = get_piece_color(piece2);
    return (color1 == color2);
}

// Pawn promotion functions
bool Game::needs_promotion(PiecePtr piece) {
    if (!piece || piece->id.length() < 2) return false;
    
    // Check if it's a pawn
    if (piece->id[0] != 'P') return false;
    
    // Only check for promotion if pawn just finished moving
    if (piece->state->name != "long_rest" && piece->state->name != "short_rest") {
        return false;
    }
    
    auto current_pos = piece->current_cell();
    char color = piece->id[1];
    
    // White pawn reaches row 0 (black's back rank)
    if (color == 'W' && current_pos.first == 0) return true;
    
    // Black pawn reaches row 7 (white's back rank)
    if (color == 'B' && current_pos.first == 7) return true;
    
    return false;
}

void Game::handle_pawn_promotion(PiecePtr pawn) {
    promoting_pawn_ = pawn;
    is_promoting_ = true;
    std::cout << "Pawn promotion! Press Q/R/B/N to choose piece type" << std::endl;
}

PiecePtr Game::create_promoted_piece(const std::string& piece_type, const std::pair<int,int>& position, char color) {
    // Create new piece ID: piece_type + color + position
    std::string new_id = piece_type + color + "_(" + std::to_string(position.first) + "," + std::to_string(position.second) + ")";
    
    // Use PieceFactory to create the new piece
    std::string pieces_root = "pieces/";
    auto img_factory = std::make_shared<OpenCvImgFactory>();
    GraphicsFactory gfx_factory(img_factory);
    PieceFactory piece_factory(board, pieces_root, gfx_factory);
    
    std::string piece_dir_name = piece_type + color;
    return piece_factory.create_piece(piece_dir_name, position);
}

Game create_game(const std::string& pieces_root, ImgFactoryPtr img_factory) {
    // Load board image
    std::string board_img_path = pieces_root + "board.png";
    std::cout << "🖼️ Trying to load board image: " << board_img_path << std::endl;
    auto board_img = img_factory->load(board_img_path, {640, 640});
    if (!board_img) {
        std::cout << "❌ Failed to load board image: " << board_img_path << std::endl;
        throw std::runtime_error("Failed to load board image: " + board_img_path);
    }
    std::cout << "✅ Board image loaded successfully" << std::endl;
    
    // Create board (8x8 chess board)
    Board board(80, 80, 8, 8, board_img);
    
    // Create graphics factory
    GraphicsFactory gfx_factory(img_factory);
    
    // Create piece factory
    PieceFactory piece_factory(board, pieces_root, gfx_factory);
    
    // Load pieces from board.csv
    std::string board_csv_path = pieces_root + "board.csv";
    auto pieces = piece_factory.create_pieces_from_board_csv(board_csv_path);
    
    return Game(pieces, board);
}
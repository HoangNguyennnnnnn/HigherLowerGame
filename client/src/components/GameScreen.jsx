// client/src/components/GameScreen.jsx (Đã sửa cho giao diện Full-Screen)
import React from 'react';

function GameScreen({ gameState, connected, loading, makeChoice, startGame }) {
    // Logic hiển thị giá trị cuối cùng (Ví dụ: khi đoán sai)
    const showFinalValue = gameState.message && gameState.message.includes("Wrong!"); 
    
    // Logic hiển thị nút New Game
    const showNewGameButton = gameState.message && gameState.message.includes("Wrong");

    return (
        <div className="app">
            
            {/* --- HEADER (STATUS & TITLE) ---
            <div className="header">
                <h1>🎮 Higher Lower Game</h1>
                <div className="status">
                    <span className={connected ? 'connected' : 'disconnected'}>
                        {connected ? '🟢 SSE Connected' : '🔴 SSE Disconnected'}
                    </span>
                </div>
            </div> */}

            {/* --- MAIN COMPARISON AREA --- */}
            <div className="game-area">
                
                {/* ITEM A (LEFT HALF) */}
                <div className="item item-A" style={{ backgroundImage: `url(${gameState.imageA})` }}>
                    <div className="item-overlay">
                        <h2>{gameState.labelA}</h2>
                        <p className="description">has</p>
                        <p className="value">
                            <span className="value-number">{gameState.valueA.toLocaleString()}</span>
                            <br /> 
                            <span className="value-description">average monthly searches</span>
                        </p>
                    </div>
                </div>

                {/* VISUAL DIVIDER/VS ELEMENT */}
                <div className="vs-circle">VS</div>

                {/* ITEM B (RIGHT HALF) - INTERACTIVE */}
                <div className="item item-B" style={{ backgroundImage: `url(${gameState.imageB})` }}>
                    <div className="item-overlay">
                        
                        <h2>{gameState.labelB}</h2>
                        <p className="description">has</p>
                        
                        {/* CHOICE BUTTONS (Centralized) */}
                        <div className="choice-buttons">
                            <button onClick={() => !loading && makeChoice(2)} disabled={loading} className="btn-higher">
                                Higher
                            </button>
                            <button onClick={() => !loading && makeChoice(1)} disabled={loading} className="btn-lower">
                                Lower
                            </button>
                            <p className="vs-text">searches than {gameState.labelA}</p>
                        </div>

                        {/* FINAL VALUE (Hiển thị khi thua) */}
                        {showFinalValue && (
                            <p className="final-value-reveal">
                                {gameState.valueB.toLocaleString()} searches
                            </p>
                        )}
                    </div>
                </div>
            </div>
            
            {/* --- STATS (FOOTER) --- */}
            <div className="stats">
                <div className="stat">
                    <span className="stat-label">Score:</span>
                    <span className="stat-value">{gameState.score}</span>
                </div>
                <div className="stat">
                    <span className="stat-label">Streak:</span>
                    <span className="stat-value">{gameState.streak}</span>
                </div>
            </div>
            
            {/* Message/Result Feedback */}
            <div className="message-feedback">{gameState.message}</div>

            {/* Nút New Game */}
            {showNewGameButton && (
                <button className="new-game-btn" onClick={startGame} disabled={loading}>
                    New Game
                </button>
            )}
        </div>
    );
}

export default GameScreen;

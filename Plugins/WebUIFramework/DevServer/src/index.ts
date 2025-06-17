
console.log("Window object keys:", Object.keys(window));
console.log("Looking for ue object:", (window as any).ue);

// Wait for the page to fully load and UE object to be bound
const waitForUE = () => {
    return new Promise<void>((resolve) => {
        const checkUE = () => {
            console.log("Checking for ue object...");
            if ((window as any).ue) {
                console.log("UE object found!", (window as any).ue);
                console.log("Available UE functions:", Object.getOwnPropertyNames((window as any).ue));
                resolve();
            } else {
                console.log("UE object not found, retrying in 100ms...");
                setTimeout(checkUE, 100);
            }
        };
        checkUE();
    });
};

// Initialize everything after UE object is available
waitForUE().then(() => {
    console.log("UE object is ready, setting up event listeners...");
    setupEventHandlers();
});

function setupEventHandlers() {
    // --- Panel Navigation ---
    const panels = document.querySelectorAll('.menu-panel');
    const showPanel = (panelId: string) => {
        panels.forEach(p => p.classList.remove('active'));
        document.getElementById(panelId)?.classList.add('active');
    };
    document.querySelectorAll('.back-btn').forEach(btn => btn.addEventListener('click', () => showPanel('main-menu')));

    // --- Main Menu Buttons (Direct Calls) ---
    document.getElementById('start-game-btn')?.addEventListener('click', () => {
        console.log("Start Game button clicked");
        console.log("UE object:", (window as any).ue);

        try {
            if ((window as any).ue?.startnewgame) {  // Note: function names are lowercase in UE
                console.log("Calling ue.startnewgame...");
                // Use a generic level name that should exist in most UE projects
                (window as any).ue.startnewgame('/Game/ThirdPerson/Maps/ThirdPersonMap');
            } else {
                console.error("ue.startnewgame not found");
                console.log("Available functions:", Object.getOwnPropertyNames((window as any).ue));
            }
        } catch (error) {
            console.error("Error calling StartNewGame:", error);
        }
    });

    document.getElementById('load-game-btn')?.addEventListener('click', () => {
        showPanel('load-menu');
        console.log("Load Game button clicked");

        try {
            if ((window as any).ue?.requestsavegameslist) {  // Note: function names are lowercase in UE
                console.log("Calling ue.requestsavegameslist...");
                (window as any).ue.requestsavegameslist();
            } else {
                console.error("ue.requestsavegameslist not found");
                console.log("Available functions:", Object.getOwnPropertyNames((window as any).ue));
            }
        } catch (error) {
            console.error("Error calling RequestSaveGamesList:", error);
        }
    });

    document.getElementById('settings-btn')?.addEventListener('click', () => showPanel('settings-menu'));

    document.getElementById('quit-game-btn')?.addEventListener('click', () => {
        console.log("Quit Game button clicked");

        try {
            if ((window as any).ue?.quitgame) {  // Note: function names are lowercase in UE
                console.log("Calling ue.quitgame...");
                (window as any).ue.quitgame();
            } else {
                console.error("ue.quitgame not found");
                console.log("Available functions:", Object.getOwnPropertyNames((window as any).ue));
            }
        } catch (error) {
            console.error("Error calling QuitGame:", error);
        }
    });

    // --- Settings Panel Logic ---
    document.getElementById('apply-settings-btn')?.addEventListener('click', () => {
        console.log("Apply Settings button clicked");

        const resolution = (document.getElementById('resolution-select') as HTMLSelectElement).value.split('x');
        const settingsPayload = {
            resolutionX: parseInt(resolution[0]),
            resolutionY: parseInt(resolution[1]),
            fullscreenMode: parseInt((document.getElementById('fullscreen-select') as HTMLSelectElement).value),
            textureQuality: parseInt((document.getElementById('texture-quality-select') as HTMLSelectElement).value),
            shadowQuality: parseInt((document.getElementById('shadow-quality-select') as HTMLSelectElement).value),
            vsync: (document.getElementById('vsync-checkbox') as HTMLInputElement).checked
        };

        console.log("Settings payload:", settingsPayload);

        try {
            if ((window as any).ue?.applyallgamesettings) {  // Note: function names are lowercase in UE
                console.log("Calling ue.applyallgamesettings...");
                (window as any).ue.applyallgamesettings(JSON.stringify(settingsPayload));
            } else {
                console.error("ue.applyallgamesettings not found");
                console.log("Available functions:", Object.getOwnPropertyNames((window as any).ue));
            }
        } catch (error) {
            console.error("Error calling ApplyAllGameSettings:", error);
        }
    });
}

// --- Save/Load Logic ---
(window as any).populateSaveSlots = (saveData: any[]) => {
    console.log("populateSaveSlots called with:", saveData);
    const container = document.getElementById('save-slots-container');
    if (!container) return;
    container.innerHTML = '';
    saveData.forEach(slot => {
        const slotButton = document.createElement('button');
        slotButton.className = 'save-slot';
        if (slot.exists) {
            slotButton.innerHTML = `<span class="slot-name">${slot.playerName || 'Used Slot'}</span><span class="slot-timestamp">${new Date(slot.timestamp).toLocaleString()}</span>`;
            slotButton.addEventListener('click', () => {
                if ((window as any).ue?.loadgame) {  // Note: function names are lowercase in UE
                    (window as any).ue.loadgame(slot.slotName);
                }
            });
        } else {
            slotButton.innerHTML = `<span class="slot-name">Empty Slot</span>`;
            slotButton.addEventListener('click', () => {
                if ((window as any).ue?.savegame) {  // Note: function names are lowercase in UE
                    (window as any).ue.savegame(slot.slotName);
                }
            });
        }
        container.appendChild(slotButton);
    });
};
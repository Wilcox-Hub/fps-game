(() => {
  const canvas = document.querySelector('#game-canvas');
  const ctx = canvas.getContext('2d');
  const $ = (selector) => document.querySelector(selector);
  const ui = {
    hud: $('#hud'), start: $('#start-screen'), pause: $('#pause-screen'), over: $('#gameover-screen'),
    health: $('#health-text'), healthFill: $('#health-fill'), healthCard: $('.health-card'),
    ammo: $('#ammo-text'), reserve: $('#reserve-text'), ammoFill: $('#ammo-fill'), score: $('#score-text'),
    wave: $('#wave-text'), objective: $('#objective'), toast: $('#toast'), hit: $('#hit-marker'),
    combo: $('#combo-text'), surge: $('#surge-meter'), surgeFill: $('#surge-fill'), surgeKey: $('#surge-key'),
    armory: $('#armory-screen'), armoryCards: $('#armory-cards'), weaponName: $('#weapon-name'),
    crosshair: $('#crosshair'), finalScore: $('#final-score'), finalWave: $('#final-wave'), best: $('#best-score')
  };
  const world = [
    '################', '#......#.......#', '#......#.......#', '#..............#',
    '#..##..........#', '#..##....##....#', '#..............#', '#.....##.......#',
    '#.....##.......#', '#..............#', '#....##.....#..#', '#....##.....#..#',
    '#..............#', '#.......#......#', '#..............#', '################'
  ];
  const spawnPoints = [
    { x: 13.5, y: 2.5 }, { x: 14.5, y: 7.5 }, { x: 12.5, y: 13.5 },
    { x: 3.5, y: 13.5 }, { x: 8.5, y: 5.5 }, { x: 10.5, y: 9.5 },
    { x: 2.5, y: 8.5 }, { x: 13.5, y: 11.5 }, { x: 6.5, y: 3.5 }
  ];
  const fixedWeapons = [
    { name: 'Gale Repeater', description: 'Fast, clean shots. Reliable until the crowd gets close.', mode: 'auto', payload: 'plain', condition: 'steady', damage: 1, cooldown: .16, magazine: 24, reserve: 144, pellets: 1 },
    { name: 'Phasma Rifle', description: 'Mirror-chrome plasma blaster. Heavy shots burst into a bassy shock ring.', mode: 'phasma', payload: 'plain', condition: 'steady', damage: 2.8, cooldown: .78, magazine: 6, reserve: 30, pellets: 1 },
  ];
  const firingModes = [
    { id: 'auto', name: 'Repeater', words: ['rapid', 'steady', 'automatic'] },
    { id: 'scatter', name: 'Scatter', words: ['wide', 'shattered', 'many-barreled'] },
    { id: 'pierce', name: 'Rail Lance', words: ['piercing', 'rail-fed', 'needlepoint'] },
    { id: 'arc', name: 'Arc Coil', words: ['chaining', 'forked', 'storm-fed'] },
    { id: 'burst', name: 'Burst Driver', words: ['triplet', 'stuttering', 'three-beat'] },
    { id: 'mortar', name: 'Mortar', words: ['lobbed', 'volatile', 'thunderous'] },
    { id: 'echo', name: 'Echo Gun', words: ['echoing', 'double-image', 'aftershock'] },
    { id: 'roulette', name: 'Misfire', words: ['unlicensed', 'unpredictable', 'questionable'] },
    { id: 'phasma', name: 'Phasma Blaster', words: ['chrome', 'seismic', 'starfall'] }
  ];
  const payloads = [
    { id: 'plain', name: 'Bare Metal', tag: 'NO PAYLOAD' },
    { id: 'burn', name: 'Sunfire', tag: 'BURN' },
    { id: 'frost', name: 'Cold Iron', tag: 'SLOW' },
    { id: 'siphon', name: 'Blood Tithe', tag: 'LIFESTEAL' },
    { id: 'salvage', name: 'Scrap Gospel', tag: 'AMMO ON KILL' },
    { id: 'volatile', name: 'Last Rites', tag: 'DEATH BLAST' },
    { id: 'ricochet', name: 'Pinball Saint', tag: 'BOUNCE' },
    { id: 'split', name: 'Many Mouths', tag: 'SPLIT' },
    { id: 'glass', name: 'Glass Verdict', tag: 'FRAGILE / HUGE HIT' },
    { id: 'leaden', name: 'Lead Sermon', tag: 'SLOW / HEAVY HIT' }
  ];
  const conditions = [
    { id: 'steady', name: 'No conditions', tag: 'ALWAYS' },
    { id: 'close', name: 'Kiss the target', tag: 'CLOSE = HUGE' },
    { id: 'longshot', name: 'Across the pit', tag: 'FAR = HUGE' },
    { id: 'sprint', name: 'Run like prey', tag: 'SPRINT = HUGE' },
    { id: 'lowhealth', name: 'Last breath', tag: 'LOW VITALS = HUGE' },
    { id: 'fullhealth', name: 'Untouched', tag: 'FULL VITALS = HUGE' },
    { id: 'lastshot', name: 'One in the chamber', tag: 'LAST ROUND = HUGE' },
    { id: 'streak', name: 'Blood remembers', tag: 'ACTIVE STREAK = HUGE' },
    { id: 'afterreload', name: 'Reload ritual', tag: 'FIRST SHOT AFTER RELOAD = HUGE' },
    { id: 'desperate', name: 'Bad odds', tag: 'LOW AMMO = HUGE' }
  ];
  const gunAdjectives = ['Saint', 'Problem', 'Promise', 'Mistake', 'Encore', 'Inheritance', 'Last Word', 'Miracle', 'Bad Idea', 'Omen', 'Gambit', 'Apology'];
  const gunNouns = ['of the Pit', 'from Nowhere', 'No. 7', 'the Unmaker', 'of Questionable Origin', 'That Hums', 'for a Friend', 'Mk. Maybe'];
  const randomItem = (items) => items[Math.floor(Math.random() * items.length)];
  let offers = [];
  let selectedWeapon = fixedWeapons[0];
  let bonusNextShot = false;
  const keys = new Set();
  const touchMoves = new Set();
  const player = { x: 2.5, y: 2.5, angle: 0, health: 100, ammo: 12, reserve: 72, score: 0, wave: 1, kits: 2, combo: 0, comboTimer: 0, surge: 0 };
  const fov = Math.PI / 2.9;
  let enemies = [];
  let waveResolving = false;
  let gameState = 'menu';
  let width = 0;
  let height = 0;
  let pixelRatio = 1;
  let lastTime = 0;
  let shotCooldown = 0;
  let reloadTime = 0;
  let fireHeld = false;
  let muzzleFlash = 0;
  let toastTimer = 0;
  let touchAimX = null;
  let soundEnabled = true;
  let audioContext;
  let bestScore = 0;
  let worldTime = 0;
  let recoil = 0;
  let damageFlash = 0;
  let pickups = [];
  let plasmaBursts = [];
  try { bestScore = Number(localStorage.getItem('ridgefire-best') || 0); } catch {}
  ui.best.textContent = 'BEST ' + String(bestScore).padStart(6, '0');

  function resize() {
    const bounds = canvas.getBoundingClientRect();
    pixelRatio = Math.min(window.devicePixelRatio || 1, 2);
    width = bounds.width;
    height = bounds.height;
    canvas.width = Math.max(1, Math.floor(width * pixelRatio));
    canvas.height = Math.max(1, Math.floor(height * pixelRatio));
    ctx.setTransform(pixelRatio, 0, 0, pixelRatio, 0, 0);
    render();
  }

  function isWall(x, y) {
    const col = Math.floor(x);
    const row = Math.floor(y);
    return row < 0 || row >= world.length || col < 0 || col >= world[0].length || world[row][col] === '#';
  }

  function movePlayer(dx, dy) {
    const radius = 0.19;
    if (!isWall(player.x + dx + Math.sign(dx) * radius, player.y)) player.x += dx;
    if (!isWall(player.x, player.y + dy + Math.sign(dy) * radius)) player.y += dy;
  }

  function normalizeAngle(angle) {
    while (angle > Math.PI) angle -= Math.PI * 2;
    while (angle < -Math.PI) angle += Math.PI * 2;
    return angle;
  }

  function castRay(angle) {
    const rayX = Math.cos(angle);
    const rayY = Math.sin(angle);
    let mapX = Math.floor(player.x);
    let mapY = Math.floor(player.y);
    const deltaX = Math.abs(1 / (rayX || 0.000001));
    const deltaY = Math.abs(1 / (rayY || 0.000001));
    const stepX = rayX < 0 ? -1 : 1;
    const stepY = rayY < 0 ? -1 : 1;
    let sideX = (rayX < 0 ? player.x - mapX : mapX + 1 - player.x) * deltaX;
    let sideY = (rayY < 0 ? player.y - mapY : mapY + 1 - player.y) * deltaY;
    let side = 0;
    let distance = 18;
    for (let count = 0; count < 64; count += 1) {
      if (sideX < sideY) { sideX += deltaX; mapX += stepX; side = 0; }
      else { sideY += deltaY; mapY += stepY; side = 1; }
      if (mapY < 0 || mapY >= world.length || mapX < 0 || mapX >= world[0].length || world[mapY][mapX] === '#') {
        distance = side === 0 ? sideX - deltaX : sideY - deltaY;
        break;
      }
    }
    const hitX = player.x + rayX * distance;
    const hitY = player.y + rayY * distance;
    return { distance: Math.max(0.12, distance * Math.cos(angle - player.angle)), side, texture: side === 0 ? hitY % 1 : hitX % 1 };
  }

  function drawWorld() {
    const sky = ctx.createLinearGradient(0, 0, 0, height * 0.6);
    sky.addColorStop(0, '#302b3a'); sky.addColorStop(0.43, '#9a5f63'); sky.addColorStop(0.78, '#e49b68'); sky.addColorStop(1, '#f0bd7e');
    ctx.fillStyle = sky; ctx.fillRect(0, 0, width, height * 0.58);
    const sunX = width * (0.68 - Math.sin(player.angle * 0.14) * 0.18);
    const sunY = height * 0.31;
    const halo = ctx.createRadialGradient(sunX, sunY, 2, sunX, sunY, height * 0.28);
    halo.addColorStop(0, '#ffe8b9'); halo.addColorStop(0.035, '#ffd79a'); halo.addColorStop(0.08, '#ffc27d66'); halo.addColorStop(1, '#ed926100');
    ctx.fillStyle = halo; ctx.fillRect(sunX - height * .28, sunY - height * .28, height * .56, height * .56);
    ctx.fillStyle = '#ffe0a8'; ctx.beginPath(); ctx.arc(sunX, sunY, Math.max(8, height * .027), 0, Math.PI * 2); ctx.fill();
    drawRidge(sunX, sunY);
    const floor = ctx.createLinearGradient(0, height * 0.48, 0, height);
    floor.addColorStop(0, '#bd8068'); floor.addColorStop(.22, '#76514d'); floor.addColorStop(1, '#241f28');
    ctx.fillStyle = floor; ctx.fillRect(0, height * 0.48, width, height * 0.52);
    ctx.save();
    ctx.globalAlpha = .12;
    ctx.strokeStyle = '#f9c18b';
    for (let line = 1; line <= 8; line += 1) {
      const y = height * .5 + (line * line / 64) * height * .5;
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
    }
    ctx.restore();
    const strip = 2;
    const depth = new Float32Array(Math.ceil(width / strip));
    for (let ray = 0; ray < depth.length; ray += 1) {
      const screenX = ray * strip;
      const hit = castRay(player.angle + (screenX / width - 0.5) * fov);
      depth[ray] = hit.distance;
      const wallHeight = Math.min(height * 1.6, height * 0.88 / hit.distance);
      const y = (height - wallHeight) / 2;
      const light = Math.max(0.12, 1 - hit.distance / 15) * (hit.side ? 0.66 : 1);
      const seam = Math.floor(Math.abs(hit.texture) * 9) % 9 === 0 ? 0.72 : 1;
      const glow = Math.round(25 + light * 55 * seam);
      ctx.fillStyle = 'rgb(' + Math.round(76 + glow * 1.2) + ',' + Math.round(48 + glow * .86) + ',' + Math.round(49 + glow * .74) + ')';
      ctx.fillRect(screenX, y, strip + 1, wallHeight);
      ctx.fillStyle = 'rgba(247,195,130,' + (light * .42) + ')';
      ctx.fillRect(screenX, y, strip + 1, Math.max(1, wallHeight * .025));
      if (seam < 1) { ctx.fillStyle = 'rgba(32,21,28,' + (0.12 + (1 - light) * .22) + ')'; ctx.fillRect(screenX, y, strip + 1, wallHeight); }
    }
    drawEnemies(depth, strip);
    drawPickups(depth, strip);
    drawPlasmaBursts(depth, strip);
    drawWeapon();
    const vignette = ctx.createRadialGradient(width / 2, height / 2, height * 0.12, width / 2, height / 2, width * 0.68);
    vignette.addColorStop(0, 'rgba(3,6,12,0)'); vignette.addColorStop(1, 'rgba(3,6,12,.56)');
    ctx.fillStyle = vignette; ctx.fillRect(0, 0, width, height);
    if (muzzleFlash > 0) { ctx.fillStyle = 'rgba(255,220,148,' + (muzzleFlash * 0.12) + ')'; ctx.fillRect(0, 0, width, height); }
    if (damageFlash > 0) { ctx.fillStyle = 'rgba(236,73,70,' + (damageFlash * .17) + ')'; ctx.fillRect(0, 0, width, height); }
  }

  function drawEnemies(depth, strip) {
    const plane = width / (2 * Math.tan(fov / 2));
    const visible = enemies.map((enemy) => {
      const dx = enemy.x - player.x;
      const dy = enemy.y - player.y;
      return { enemy, distance: Math.hypot(dx, dy), angle: normalizeAngle(Math.atan2(dy, dx) - player.angle) };
    }).filter((item) => Math.abs(item.angle) < fov * 0.72 && item.distance > 0.2).sort((a, b) => b.distance - a.distance);
    for (const item of visible) {
      const screenX = width / 2 + Math.tan(item.angle) * plane;
      const scale = Math.min(height * 0.72, height * 0.7 / item.distance);
      const spriteWidth = scale * (item.enemy.kind === 'brute' ? 0.74 : 0.52);
      const ray = Math.max(0, Math.min(depth.length - 1, Math.floor(screenX / strip)));
      if (depth[ray] < item.distance - 0.18) continue;
      const bob = Math.sin(worldTime * 4 + enemy.x * 3) * scale * .035;
      const ground = height * 0.52 + scale * 0.32 + bob;
      const top = ground - scale * 0.82;
      const enemy = item.enemy;
      ctx.save();
      const armor = enemy.hit > 0 ? '#fff1ce' : (enemy.kind === 'brute' ? '#6c514d' : '#36444a');
      const glow = enemy.hit > 0 ? '#fff1ce' : (enemy.kind === 'brute' ? '#ff785f' : '#8be4dc');
      ctx.shadowColor = glow; ctx.shadowBlur = Math.max(7, scale * .2);
      ctx.fillStyle = '#29252b';
      ctx.beginPath(); ctx.ellipse(screenX, ground + scale * .02, spriteWidth * .4, scale * .07, 0, 0, Math.PI * 2); ctx.fill();
      ctx.strokeStyle = '#342e33'; ctx.lineWidth = Math.max(2, scale * .045);
      ctx.beginPath(); ctx.moveTo(screenX - spriteWidth * .23, top + scale * .35); ctx.lineTo(screenX - spriteWidth * .5, top + scale * .44); ctx.lineTo(screenX - spriteWidth * .61, top + scale * .25); ctx.moveTo(screenX + spriteWidth * .23, top + scale * .35); ctx.lineTo(screenX + spriteWidth * .5, top + scale * .44); ctx.lineTo(screenX + spriteWidth * .61, top + scale * .25); ctx.stroke();
      ctx.fillStyle = '#5c6462';
      ctx.beginPath(); ctx.ellipse(screenX - spriteWidth * .51, top + scale * .25, spriteWidth * .12, scale * .065, -.28, 0, Math.PI * 2); ctx.ellipse(screenX + spriteWidth * .51, top + scale * .25, spriteWidth * .12, scale * .065, .28, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = armor;
      ctx.beginPath(); ctx.moveTo(screenX - spriteWidth * .43, top + scale * .36); ctx.lineTo(screenX - spriteWidth * .29, top + scale * .15); ctx.lineTo(screenX - spriteWidth * .13, top + scale * .21); ctx.lineTo(screenX, top + scale * .16); ctx.lineTo(screenX + spriteWidth * .13, top + scale * .21); ctx.lineTo(screenX + spriteWidth * .29, top + scale * .15); ctx.lineTo(screenX + spriteWidth * .43, top + scale * .36); ctx.lineTo(screenX + spriteWidth * .3, top + scale * .57); ctx.lineTo(screenX, top + scale * .63); ctx.lineTo(screenX - spriteWidth * .3, top + scale * .57); ctx.closePath(); ctx.fill();
      ctx.strokeStyle = '#b99c77'; ctx.lineWidth = Math.max(1, scale * .022); ctx.stroke();
      ctx.fillStyle = '#23252a'; ctx.beginPath(); ctx.ellipse(screenX, top + scale * .31, spriteWidth * .22, scale * .115, 0, Math.PI, Math.PI * 2); ctx.fill();
      ctx.fillStyle = glow; ctx.shadowBlur = Math.max(10, scale * .28); ctx.beginPath(); ctx.ellipse(screenX, top + scale * .335, spriteWidth * .135, Math.max(2, scale * .035), 0, 0, Math.PI * 2); ctx.fill();
      ctx.shadowBlur = 0;
      ctx.fillStyle = '#f2a66d'; ctx.beginPath(); ctx.moveTo(screenX - spriteWidth * .12, top + scale * .62); ctx.lineTo(screenX, top + scale * .72 + Math.sin(worldTime * 20 + enemy.phase) * scale * .012); ctx.lineTo(screenX + spriteWidth * .12, top + scale * .62); ctx.closePath(); ctx.fill();
      ctx.restore();
      const barWidth = spriteWidth * 0.72;
      ctx.fillStyle = '#04070bb3'; ctx.fillRect(screenX - barWidth / 2, top - 8, barWidth, 3);
      ctx.fillStyle = '#ff785f'; ctx.fillRect(screenX - barWidth / 2, top - 8, barWidth * Math.max(0, enemy.hp / enemy.maxHp), 3);
    }
  }

  function drawRidge(sunX, sunY) {
    const horizon = height * .51;
    const shift = player.angle * width * .15;
    const ridges = [
      { color: '#86555a', offset: .02, peaks: [.04,.05,.12,.39,.18,.08,.28,.04,.17,.03] },
      { color: '#68474f', offset: .055, peaks: [.1,.03,.26,.09,.14,.04,.32,.08,.18,.02] },
      { color: '#473b46', offset: .09, peaks: [.16,.02,.22,.06,.11,.03,.27,.08,.2,.04] }
    ];
    for (const ridge of ridges) {
      ctx.fillStyle = ridge.color; ctx.beginPath(); ctx.moveTo(-width, height);
      ridge.peaks.forEach((peak, index) => ctx.lineTo(index / (ridge.peaks.length - 1) * width * 2 - shift * ridge.offset, horizon - height * peak));
      ctx.lineTo(width * 2, height); ctx.closePath(); ctx.fill();
    }
    ctx.fillStyle = '#e59b6b'; ctx.globalAlpha = .18;
    ctx.beginPath(); ctx.ellipse(sunX, sunY + height * .04, width * .16, height * .035, 0, 0, Math.PI * 2); ctx.fill(); ctx.globalAlpha = 1;
    ctx.fillStyle = '#40333d';
    ctx.fillRect(width * .08 - shift * .04, horizon - height * .11, width * .055, height * .12);
    ctx.fillRect(width * .13 - shift * .04, horizon - height * .16, width * .018, height * .17);
    ctx.fillRect(width * .12 - shift * .04, horizon - height * .18, width * .04, height * .028);
  }

  function drawPickups(depth, strip) {
    const plane = width / (2 * Math.tan(fov / 2));
    for (const pickup of pickups) {
      const dx = pickup.x - player.x; const dy = pickup.y - player.y;
      const distance = Math.hypot(dx, dy); const angle = normalizeAngle(Math.atan2(dy, dx) - player.angle);
      if (Math.abs(angle) > fov * .62 || distance < .15) continue;
      const screenX = width / 2 + Math.tan(angle) * plane;
      const ray = Math.max(0, Math.min(depth.length - 1, Math.floor(screenX / strip)));
      if (depth[ray] < distance - .15) continue;
      const size = Math.min(height * .18, height * .22 / distance);
      const centerY = height * .57 - size * .25 + Math.sin(worldTime * 4 + pickup.x) * size * .06;
      ctx.save(); ctx.shadowColor = pickup.type === 'health' ? '#ff785f' : '#8be4dc'; ctx.shadowBlur = size * .3;
      ctx.fillStyle = pickup.type === 'health' ? '#ff785f' : '#8be4dc';
      ctx.beginPath(); ctx.arc(screenX, centerY, size * .34, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = '#28242b'; ctx.beginPath(); ctx.arc(screenX, centerY, size * .23, 0, Math.PI * 2); ctx.fill();
      ctx.fillStyle = '#fff1ce'; ctx.font = 'bold ' + Math.max(8, size * .34) + 'px Arial'; ctx.textAlign = 'center'; ctx.textBaseline = 'middle'; ctx.fillText(pickup.type === 'health' ? '+' : '⌁', screenX, centerY);
      ctx.restore();
    }
  }

  function drawPlasmaBursts(depth, strip) {
    const plane = width / (2 * Math.tan(fov / 2));
    for (const burst of plasmaBursts) {
      const dx = burst.x - player.x; const dy = burst.y - player.y;
      const distance = Math.hypot(dx, dy); const angle = normalizeAngle(Math.atan2(dy, dx) - player.angle);
      if (Math.abs(angle) > fov * .62 || distance < .15) continue;
      const screenX = width / 2 + Math.tan(angle) * plane;
      const ray = Math.max(0, Math.min(depth.length - 1, Math.floor(screenX / strip)));
      if (depth[ray] < distance - .15) continue;
      const size = Math.min(height * .26, height * .3 / distance);
      const centerY = height * .54 - size * .17;
      const fade = Math.max(0, burst.life / burst.duration);
      const spread = 1 - fade;
      ctx.save(); ctx.globalAlpha = fade;
      ctx.shadowColor = '#9beeff'; ctx.shadowBlur = size * .3;
      ctx.strokeStyle = '#f4fdff'; ctx.lineWidth = Math.max(2, size * .035);
      ctx.beginPath(); ctx.ellipse(screenX, centerY, size * (.12 + spread * .55), size * (.1 + spread * .3), 0, 0, Math.PI * 2); ctx.stroke();
      ctx.strokeStyle = '#6ac9ed'; ctx.lineWidth = Math.max(1, size * .018);
      ctx.beginPath(); ctx.ellipse(screenX, centerY, size * (.22 + spread * .76), size * (.17 + spread * .45), 0, 0, Math.PI * 2); ctx.stroke();
      ctx.fillStyle = '#ecfcff'; ctx.beginPath(); ctx.arc(screenX, centerY, size * fade * .12, 0, Math.PI * 2); ctx.fill();
      ctx.restore();
    }
  }

  function drawWeapon() {
    const sway = Math.sin(worldTime * 7) * 3;
    const kick = recoil * 18;
    const x = width * .5; const y = height * 1.08 + kick;
    ctx.save(); ctx.translate(x + sway, y); ctx.scale(Math.max(.72, width / 1000), Math.max(.72, height / 650));
    if (selectedWeapon.mode === 'phasma') {
      const chrome = ctx.createLinearGradient(-112, -120, 105, 20);
      chrome.addColorStop(0, '#4b5663'); chrome.addColorStop(.19, '#eaf2f7'); chrome.addColorStop(.35, '#899aa8'); chrome.addColorStop(.52, '#fbffff'); chrome.addColorStop(.7, '#758493'); chrome.addColorStop(.88, '#d7e1e7'); chrome.addColorStop(1, '#43515d');
      ctx.shadowColor = '#18202a'; ctx.shadowBlur = 24;
      ctx.fillStyle = '#262c35'; ctx.beginPath(); ctx.moveTo(-122, 30); ctx.lineTo(-92, -83); ctx.lineTo(-71, -125); ctx.lineTo(71, -125); ctx.lineTo(95, -78); ctx.lineTo(124, 30); ctx.lineTo(63, 42); ctx.lineTo(-70, 40); ctx.closePath(); ctx.fill();
      ctx.shadowBlur = 0; ctx.fillStyle = chrome;
      ctx.beginPath(); ctx.moveTo(-105, 23); ctx.lineTo(-78, -82); ctx.lineTo(-58, -112); ctx.lineTo(59, -112); ctx.lineTo(80, -78); ctx.lineTo(105, 22); ctx.lineTo(53, 31); ctx.lineTo(-55, 31); ctx.closePath(); ctx.fill();
      ctx.fillStyle = '#252d36'; ctx.fillRect(-27, -146, 54, 62);
      ctx.fillStyle = chrome; ctx.fillRect(-20, -165, 40, 93);
      ctx.fillStyle = '#111d29'; ctx.fillRect(-10, -162, 20, 75);
      ctx.strokeStyle = '#91eaff'; ctx.lineWidth = 4; ctx.shadowColor = '#66cfff'; ctx.shadowBlur = 16;
      ctx.beginPath(); ctx.moveTo(-37, -100); ctx.lineTo(-32, -51); ctx.lineTo(-22, -29); ctx.moveTo(37, -100); ctx.lineTo(32, -51); ctx.lineTo(22, -29); ctx.stroke();
      ctx.shadowBlur = 0; ctx.fillStyle = '#74d6f4'; ctx.fillRect(-42, -75, 8, 39); ctx.fillRect(34, -75, 8, 39);
      ctx.fillStyle = '#3b4854'; ctx.fillRect(-17, -48, 34, 9); ctx.fillStyle = chrome; ctx.fillRect(-12, -43, 24, 8);
      ctx.fillStyle = '#242c35'; ctx.fillRect(-17, -31, 34, 63);
      ctx.fillStyle = chrome; ctx.fillRect(-12, -29, 24, 53);
      ctx.fillStyle = '#a6f1ff'; ctx.shadowColor = '#57d5ff'; ctx.shadowBlur = 20; ctx.beginPath(); ctx.ellipse(0, -99, 12, 28, 0, 0, Math.PI * 2); ctx.fill(); ctx.shadowBlur = 0;
      ctx.fillStyle = '#f3fcff'; ctx.fillRect(-4, -153, 8, 47);
      if (muzzleFlash > .05) { ctx.fillStyle = '#effcff'; ctx.shadowColor = '#66d9ff'; ctx.shadowBlur = 34; ctx.beginPath(); ctx.moveTo(-22, -161); ctx.lineTo(-14, -211 - muzzleFlash * 19); ctx.lineTo(0, -190); ctx.lineTo(14, -220 - muzzleFlash * 12); ctx.lineTo(22, -161); ctx.closePath(); ctx.fill(); }
      ctx.restore();
      return;
    }
    ctx.shadowColor = '#11121a'; ctx.shadowBlur = 20;
    ctx.fillStyle = '#302d32'; ctx.beginPath(); ctx.moveTo(-118, 30); ctx.lineTo(-76, -118); ctx.lineTo(-35, -139); ctx.lineTo(42, -136); ctx.lineTo(83, -104); ctx.lineTo(124, 31); ctx.closePath(); ctx.fill();
    ctx.shadowBlur = 0;
    ctx.fillStyle = '#74645a'; ctx.beginPath(); ctx.moveTo(-104, 28); ctx.lineTo(-66, -101); ctx.lineTo(-30, -123); ctx.lineTo(35, -121); ctx.lineTo(68, -91); ctx.lineTo(106, 29); ctx.closePath(); ctx.fill();
    ctx.fillStyle = '#39343a'; ctx.beginPath(); ctx.moveTo(-55, -90); ctx.lineTo(55, -90); ctx.lineTo(77, 33); ctx.lineTo(-78, 33); ctx.closePath(); ctx.fill();
    ctx.fillStyle = '#181a20'; ctx.beginPath(); ctx.moveTo(-31, -111); ctx.lineTo(-22, -147); ctx.lineTo(22, -147); ctx.lineTo(31, -111); ctx.closePath(); ctx.fill();
    ctx.strokeStyle = '#8be4dc'; ctx.lineWidth = 3; ctx.shadowColor = '#8be4dc'; ctx.shadowBlur = 12; ctx.beginPath(); ctx.moveTo(-17, -119); ctx.lineTo(-11, -140); ctx.lineTo(11, -140); ctx.lineTo(17, -119); ctx.stroke(); ctx.shadowBlur = 0;
    ctx.fillStyle = '#ba8564'; ctx.fillRect(-17, -77, 34, 5);
    ctx.fillStyle = '#1b1c22'; ctx.fillRect(-6, -56, 12, 76);
    ctx.fillStyle = '#d69c68'; ctx.fillRect(-52, 8, 16, 19); ctx.fillRect(36, 8, 16, 19);
    if (muzzleFlash > .05) { ctx.fillStyle = '#ffe2a1'; ctx.shadowColor = '#ff9d63'; ctx.shadowBlur = 26; ctx.beginPath(); ctx.moveTo(-17, -146); ctx.lineTo(-9, -181 - muzzleFlash * 18); ctx.lineTo(0, -163); ctx.lineTo(11, -189 - muzzleFlash * 13); ctx.lineTo(18, -145); ctx.closePath(); ctx.fill(); }
    ctx.restore();
  }

  function showToast(message) {
    ui.toast.textContent = message;
    ui.toast.classList.add('show');
    toastTimer = 1.4;
  }

  function playTone(frequency, duration, type, volume) {
    if (!soundEnabled) return;
    try {
      audioContext ||= new AudioContext();
      if (audioContext.state === 'suspended') audioContext.resume();
      const oscillator = audioContext.createOscillator();
      const gain = audioContext.createGain();
      oscillator.type = type || 'sine';
      oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
      gain.gain.setValueAtTime(volume || 0.035, audioContext.currentTime);
      gain.gain.exponentialRampToValueAtTime(0.001, audioContext.currentTime + duration);
      oscillator.connect(gain).connect(audioContext.destination);
      oscillator.start(); oscillator.stop(audioContext.currentTime + duration);
    } catch {}
  }

  function playPhasmaShot() {
    if (!soundEnabled) return;
    try {
      audioContext ||= new AudioContext();
      if (audioContext.state === 'suspended') audioContext.resume();
      const now = audioContext.currentTime;
      const sweep = audioContext.createOscillator();
      const sweepGain = audioContext.createGain();
      sweep.type = 'sine';
      sweep.frequency.setValueAtTime(390, now);
      sweep.frequency.exponentialRampToValueAtTime(58, now + .34);
      sweepGain.gain.setValueAtTime(.001, now);
      sweepGain.gain.linearRampToValueAtTime(.13, now + .035);
      sweepGain.gain.exponentialRampToValueAtTime(.001, now + .52);
      sweep.connect(sweepGain).connect(audioContext.destination);
      sweep.start(now); sweep.stop(now + .54);

      const impact = audioContext.createOscillator();
      const impactGain = audioContext.createGain();
      impact.type = 'triangle';
      impact.frequency.setValueAtTime(102, now + .22);
      impact.frequency.exponentialRampToValueAtTime(31, now + .72);
      impactGain.gain.setValueAtTime(.001, now + .21);
      impactGain.gain.linearRampToValueAtTime(.23, now + .245);
      impactGain.gain.exponentialRampToValueAtTime(.001, now + .92);
      impact.connect(impactGain).connect(audioContext.destination);
      impact.start(now + .21); impact.stop(now + .94);

      const ring = audioContext.createOscillator();
      const ringGain = audioContext.createGain();
      const delay = audioContext.createDelay();
      const echoGain = audioContext.createGain();
      ring.type = 'sine';
      ring.frequency.setValueAtTime(720, now + .29);
      ring.frequency.exponentialRampToValueAtTime(190, now + .66);
      ringGain.gain.setValueAtTime(.001, now + .28);
      ringGain.gain.linearRampToValueAtTime(.055, now + .31);
      ringGain.gain.exponentialRampToValueAtTime(.001, now + .82);
      delay.delayTime.setValueAtTime(.12, now);
      echoGain.gain.setValueAtTime(.24, now);
      ring.connect(ringGain);
      ringGain.connect(audioContext.destination);
      ringGain.connect(delay);
      delay.connect(echoGain);
      echoGain.connect(delay);
      delay.connect(audioContext.destination);
      ring.start(now + .28); ring.stop(now + .84);

      const noiseLength = Math.floor(audioContext.sampleRate * .22);
      const noiseBuffer = audioContext.createBuffer(1, noiseLength, audioContext.sampleRate);
      const noiseData = noiseBuffer.getChannelData(0);
      for (let sample = 0; sample < noiseLength; sample += 1) noiseData[sample] = (Math.random() * 2 - 1) * (1 - sample / noiseLength);
      const noise = audioContext.createBufferSource();
      const filter = audioContext.createBiquadFilter();
      const noiseGain = audioContext.createGain();
      noise.buffer = noiseBuffer; filter.type = 'lowpass'; filter.frequency.setValueAtTime(1450, now + .22); filter.frequency.exponentialRampToValueAtTime(340, now + .44);
      noiseGain.gain.setValueAtTime(.001, now + .21); noiseGain.gain.linearRampToValueAtTime(.12, now + .235); noiseGain.gain.exponentialRampToValueAtTime(.001, now + .48);
      noise.connect(filter).connect(noiseGain).connect(audioContext.destination);
      noise.start(now + .21); noise.stop(now + .49);
    } catch {}
  }

  function updateHud() {
    ui.health.textContent = String(Math.ceil(player.health));
    ui.healthFill.style.width = Math.max(0, player.health) + '%';
    ui.healthCard.classList.toggle('low', player.health <= 30);
    ui.ammo.textContent = String(player.ammo).padStart(2, '0');
    ui.reserve.textContent = String(player.reserve).padStart(2, '0');
    ui.ammoFill.style.width = (player.ammo / selectedWeapon.magazine) * 100 + '%';
    ui.weaponName.textContent = selectedWeapon.name.toUpperCase();
    ui.score.textContent = String(player.score).padStart(6, '0');
    ui.combo.textContent = player.combo > 1 ? player.combo + ' STREAK · x' + Math.min(8, 1 + Math.floor(player.combo / 3)) : 'BUILD A STREAK';
    ui.surgeFill.style.width = player.surge + '%';
    ui.surgeKey.textContent = player.surge >= 100 ? 'E · READY' : 'E · ' + player.surge + '%';
    ui.surge.classList.toggle('ready', player.surge >= 100);
  }

  function rollWildcard() {
    const mode = randomItem(firingModes);
    const payload = randomItem(payloads.slice(1));
    const condition = Math.random() < .76 ? randomItem(conditions.slice(1)) : conditions[0];
    const damageScale = randomItem([.25, .4, .65, .8, 1, 1.2, 1.6, 2.4, 4, 7]);
    const tempoScale = randomItem([.38, .55, .75, 1, 1, 1.25, 1.8, 2.7]);
    const magazine = randomItem([3, 5, 8, 13, 21, 34]);
    const name = randomItem(mode.words) + ' ' + randomItem(gunAdjectives) + ' ' + randomItem(gunNouns);
    const parts = [mode.name, payload.name];
    if (condition.id !== 'steady') parts.push(condition.name);
    return {
      name,
      description: parts.join(' + ') + '. Rolled stats: ' + damageScale + '× damage, ' + tempoScale + '× delay, ' + magazine + ' rounds. The pit makes no promises.',
      mode: mode.id,
      payload: payload.id,
      condition: condition.id,
      damage: damageScale,
      cooldown: Math.max(.055, .19 * tempoScale),
      magazine,
      reserve: magazine * randomItem([2, 4, 6, 9]),
      pellets: randomItem([3, 5, 7, 11]),
      wildcard: true
    };
  }

  function renderArmory() {
    offers = [...fixedWeapons, rollWildcard()];
    ui.armoryCards.replaceChildren();
    offers.forEach((weapon, index) => {
      const card = document.createElement('button');
      card.type = 'button';
      card.className = 'weapon-card' + (weapon.wildcard ? ' wildcard' : '') + (weapon.mode === 'phasma' ? ' phasma' : '');
      const badge = document.createElement('span');
      badge.className = 'weapon-index';
      badge.textContent = weapon.wildcard ? '03 // WILDCARD' : '0' + (index + 1) + ' // TRIED & TESTED';
      card.append(badge);
      if (weapon.wildcard) {
        const roll = document.createElement('span');
        roll.className = 'wild-roll';
        roll.textContent = 'FRESH ROLL';
        card.append(roll);
      }
      const title = document.createElement('strong'); title.textContent = weapon.name;
      const description = document.createElement('p'); description.textContent = weapon.description;
      const tags = document.createElement('div'); tags.className = 'weapon-tags';
      const mode = firingModes.find((item) => item.id === weapon.mode);
      [mode.name, payloads.find((item) => item.id === weapon.payload).tag, conditions.find((item) => item.id === weapon.condition).tag].forEach((label) => {
        const tag = document.createElement('span'); tag.textContent = label; tags.append(tag);
      });
      card.append(title, description, tags);
      card.addEventListener('click', () => resetRun(weapon));
      ui.armoryCards.append(card);
    });
  }

  function openArmory() {
    renderArmory();
    setState('armory');
  }

  function setState(state) {
    gameState = state;
    ui.start.hidden = state !== 'menu';
    ui.armory.hidden = state !== 'armory';
    ui.pause.hidden = state !== 'paused';
    ui.over.hidden = state !== 'gameover';
    ui.hud.hidden = state !== 'playing';
    if (state !== 'playing') {
      fireHeld = false;
      if (document.pointerLockElement === canvas) document.exitPointerLock();
    }
  }

  function spawnWave() {
    waveResolving = false;
    const count = Math.min(3 + player.wave * 2, 14);
    const points = [...spawnPoints].sort(() => Math.random() - 0.5);
    enemies = [];
    for (let index = 0; index < count; index += 1) {
      const point = points[index % points.length];
      const kind = player.wave >= 2 && index % 4 === 2 ? 'skimmer' : player.wave >= 3 && index % 4 === 3 ? 'brute' : 'drone';
      const hp = kind === 'brute' ? 4 + Math.floor(player.wave / 2) : kind === 'skimmer' ? 2 + Math.floor(player.wave / 4) : 2 + Math.floor(player.wave / 3);
      enemies.push({ x: point.x, y: point.y, hp, maxHp: hp, kind, speed: kind === 'brute' ? 0.42 : kind === 'skimmer' ? 0.78 : 0.66, cooldown: 0.5 + Math.random(), hit: 0, phase: Math.random() * Math.PI * 2 });
    }
    ui.wave.textContent = String(player.wave).padStart(2, '0');
    ui.objective.textContent = 'WAVE ' + String(player.wave).padStart(2, '0') + ' // ' + count + ' SENTINELS';
    showToast(player.wave === 1 ? 'THE CROWD WANTS BLOOD' : 'WAVE ' + player.wave + ' // HOLD THE PIT');
  }

  function requestPointerLock() {
    if (canvas.requestPointerLock && !matchMedia('(pointer: coarse)').matches) {
      try { const pending = canvas.requestPointerLock(); if (pending && pending.catch) pending.catch(() => {}); } catch {}
    }
  }

  function resetRun(weapon = selectedWeapon) {
    selectedWeapon = weapon;
    player.x = 2.5; player.y = 2.5; player.angle = 0;
    player.health = 100; player.ammo = selectedWeapon.magazine; player.reserve = selectedWeapon.reserve;
    player.score = 0; player.wave = 1; player.kits = 2;
    player.combo = 0; player.comboTimer = 0; player.surge = 0; pickups = []; plasmaBursts = [];
    bonusNextShot = false;
    reloadTime = 0; shotCooldown = 0;
    updateHud(); spawnWave(); setState('playing'); requestPointerLock();
  }

  function finishRun() {
    setState('gameover');
    ui.finalScore.textContent = String(player.score).padStart(6, '0');
    ui.finalWave.textContent = String(Math.max(0, player.wave - 1)).padStart(2, '0');
    if (player.score > bestScore) {
      bestScore = player.score;
      try { localStorage.setItem('ridgefire-best', String(bestScore)); } catch {}
      ui.best.textContent = 'BEST ' + String(bestScore).padStart(6, '0');
    }
    playTone(110, 0.5, 'triangle', 0.06);
  }

  function clearWave() {
    player.wave += 1; player.reserve += 24; player.health = Math.min(100, player.health + 12);
    if (player.wave % 3 === 0) player.kits = Math.min(3, player.kits + 1);
    player.surge = Math.min(100, player.surge + 18);
    updateHud(); showToast('RIDGE HELD // +24 CHARGE · +12 VITALS');
    window.setTimeout(() => { if (gameState === 'playing') spawnWave(); }, 1050);
  }

  function targetAtAngle(angle, toleranceScale = 1) {
    const wallDistance = castRay(angle).distance;
    let target = null; let targetDistance = Infinity;
    for (const enemy of enemies) {
      const dx = enemy.x - player.x; const dy = enemy.y - player.y;
      const distance = Math.hypot(dx, dy);
      const error = Math.abs(normalizeAngle(Math.atan2(dy, dx) - angle));
      const tolerance = Math.max(.045, Math.atan2(enemy.kind === 'brute' ? .42 : .3, distance)) * toleranceScale;
      if (error < tolerance && distance < wallDistance + .16 && distance < targetDistance) { target = enemy; targetDistance = distance; }
    }
    return target ? { enemy: target, distance: targetDistance } : null;
  }

  function damageFor(gun, enemy, distance) {
    let damage = gun.damage;
    switch (gun.condition) {
      case 'close': damage *= distance < 2.4 ? 4 : .35; break;
      case 'longshot': damage *= distance > 7 ? 3.5 : .45; break;
      case 'sprint': damage *= keys.has('ShiftLeft') ? 2.8 : .5; break;
      case 'lowhealth': damage *= player.health < 28 ? 4.5 : .6; break;
      case 'fullhealth': damage *= player.health > 85 ? 3 : .4; break;
      case 'lastshot': damage *= player.ammo === 0 ? 7 : .65; break;
      case 'streak': damage *= player.combo >= 3 && player.comboTimer > 0 ? 3 : .65; break;
      case 'afterreload': damage *= bonusNextShot ? 5 : .55; break;
      case 'desperate': damage *= player.ammo <= 2 ? 4 : .55; break;
    }
    if (gun.payload === 'glass') damage *= randomItem([.12, .3, .65, 1.4, 3, 8]);
    if (gun.payload === 'leaden') damage *= .38;
    if (enemy.kind === 'brute') damage *= .8;
    return damage;
  }

  function damageEnemy(enemy, damage, gun) {
    if (!enemies.includes(enemy)) return false;
    enemy.hp -= damage; enemy.hit = .18;
    if (gun?.payload === 'burn') { enemy.burnTimer = 2.4; enemy.burnTick = .35; enemy.burnDamage = Math.max(enemy.burnDamage || 0, damage * .28); }
    if (gun?.payload === 'frost') { enemy.frostTimer = 2.5; }
    if (gun?.payload === 'siphon') player.health = Math.min(100, player.health + .8);
    if (enemy.hp <= 0) { destroyEnemy(enemy, gun); return true; }
    return false;
  }

  function shoot() {
    if (gameState !== 'playing' || shotCooldown > 0 || reloadTime > 0) return;
    if (player.ammo <= 0) { showToast(player.reserve > 0 ? 'RELOAD // R' : 'EMPTY // FIND A CACHE'); shotCooldown = 0.2; return; }
    const gun = selectedWeapon;
    player.ammo -= 1; shotCooldown = gun.cooldown; muzzleFlash = 1; recoil = 1;
    ui.crosshair.classList.add('firing'); window.setTimeout(() => ui.crosshair.classList.remove('firing'), 75);
    if (gun.mode === 'phasma') playPhasmaShot();
    else playTone(72 + Math.random() * 40, gun.mode === 'mortar' ? .22 : .11, gun.mode === 'arc' ? 'triangle' : 'sawtooth', .055);
    const hits = [];
    const main = targetAtAngle(player.angle, gun.mode === 'scatter' ? 1.65 : gun.mode === 'roulette' ? 2 : 1);
    if (gun.mode === 'phasma' && main) {
      const blastDamage = damageFor(gun, main.enemy, main.distance);
      damageEnemy(main.enemy, blastDamage, gun); hits.push(main.enemy);
      plasmaBursts.push({ x: main.enemy.x, y: main.enemy.y, life: .42, duration: .42 });
      for (const enemy of [...enemies]) {
        if (enemy === main.enemy) continue;
        const distance = Math.hypot(enemy.x - main.enemy.x, enemy.y - main.enemy.y);
        if (distance < 2.6) { damageEnemy(enemy, blastDamage * .72, gun); hits.push(enemy); }
      }
    } else if (gun.mode === 'scatter') {
      for (let pellet = 0; pellet < gun.pellets; pellet += 1) {
        const shot = targetAtAngle(player.angle + (Math.random() - .5) * .2, 1.35);
        if (shot) { damageEnemy(shot.enemy, damageFor(gun, shot.enemy, shot.distance) * .34, gun); hits.push(shot.enemy); }
      }
    } else if (gun.mode === 'pierce' || gun.mode === 'echo') {
      const targets = enemies.map((enemy) => ({ enemy, distance: Math.hypot(enemy.x - player.x, enemy.y - player.y), error: Math.abs(normalizeAngle(Math.atan2(enemy.y - player.y, enemy.x - player.x) - player.angle)) }))
        .filter((target) => target.error < Math.max(.035, Math.atan2(.3, target.distance)) && target.distance < castRay(player.angle).distance + .15)
        .sort((first, second) => first.distance - second.distance);
      const count = gun.mode === 'echo' ? 2 : targets.length;
      targets.slice(0, count).forEach((target, index) => { damageEnemy(target.enemy, damageFor(gun, target.enemy, target.distance) * (index ? .72 : 1), gun); hits.push(target.enemy); });
    } else if (gun.mode === 'arc' && main) {
      let source = main.enemy; let range = 2.3;
      for (let jump = 0; jump < 4 && source; jump += 1) {
        damageEnemy(source, damageFor(gun, source, Math.hypot(source.x - player.x, source.y - player.y)) * Math.pow(.72, jump), gun); hits.push(source);
        const prior = source;
        source = enemies.filter((enemy) => enemy !== prior && !hits.includes(enemy) && Math.hypot(enemy.x - prior.x, enemy.y - prior.y) < range).sort((first, second) => Math.hypot(first.x - prior.x, first.y - prior.y) - Math.hypot(second.x - prior.x, second.y - prior.y))[0];
      }
    } else if (gun.mode === 'burst' && main) {
      for (let round = 0; round < 3; round += 1) { damageEnemy(main.enemy, damageFor(gun, main.enemy, main.distance) * .72, gun); hits.push(main.enemy); }
    } else if (gun.mode === 'mortar' && main) {
      for (const enemy of [...enemies]) {
        const distance = Math.hypot(enemy.x - main.enemy.x, enemy.y - main.enemy.y);
        if (distance < 2.5) { damageEnemy(enemy, damageFor(gun, enemy, main.distance) * (enemy === main.enemy ? 1 : .7), gun); hits.push(enemy); }
      }
    } else if (gun.mode === 'roulette') {
      const shot = targetAtAngle(player.angle + (Math.random() - .5) * .5, 1.8);
      if (shot) { damageEnemy(shot.enemy, damageFor(gun, shot.enemy, shot.distance) * randomItem([.1, .4, 1, 2, 5, 12]), gun); hits.push(shot.enemy); }
    } else if (main) {
      damageEnemy(main.enemy, damageFor(gun, main.enemy, main.distance), gun); hits.push(main.enemy);
    }
    if (gun.payload === 'ricochet' || gun.payload === 'split') {
      const alreadyHit = new Set(hits);
      for (const hit of hits) {
        const next = enemies.filter((enemy) => !alreadyHit.has(enemy) && Math.hypot(enemy.x - hit.x, enemy.y - hit.y) < (gun.payload === 'split' ? 3 : 2)).sort((first, second) => Math.hypot(first.x - hit.x, first.y - hit.y) - Math.hypot(second.x - hit.x, second.y - hit.y))[0];
        if (next) { alreadyHit.add(next); damageEnemy(next, damageFor(gun, next, Math.hypot(next.x - player.x, next.y - player.y)) * .62, null); hits.push(next); }
      }
    }
    if (hits.length) {
      ui.hit.classList.add('visible'); window.setTimeout(() => ui.hit.classList.remove('visible'), 180);
      playTone(480, 0.07, 'triangle', 0.045);
    }
    if (gun.condition === 'afterreload') bonusNextShot = false;
    if (gun.payload === 'leaden' && Math.random() < .15) { player.health = Math.max(1, player.health - 1); showToast('LEAD WEIGHT // BLOOD PRICE'); }
    updateHud();
  }

  function reload() {
    if (gameState !== 'playing' || reloadTime > 0 || player.ammo === selectedWeapon.magazine || player.reserve <= 0) return;
    reloadTime = 1.05; showToast('RECHARGING');
  }

  function medkit() {
    if (gameState !== 'playing' || player.kits <= 0 || player.health >= 100) return;
    player.kits -= 1; player.health = Math.min(100, player.health + 38);
    showToast('PATCHED UP // ' + player.kits + ' MED LEFT'); playTone(390, 0.18, 'sine', 0.04); updateHud();
  }

  function destroyEnemy(enemy, sourceGun = selectedWeapon) {
    if (!enemies.includes(enemy)) return;
    enemies = enemies.filter((item) => item !== enemy);
    player.combo = player.comboTimer > 0 ? player.combo + 1 : 1;
    player.comboTimer = 2.7;
    const multiplier = Math.min(8, 1 + Math.floor(player.combo / 3));
    const points = enemy.kind === 'brute' ? 260 : enemy.kind === 'skimmer' ? 160 : 110;
    player.score += points * multiplier;
    player.surge = Math.min(100, player.surge + (enemy.kind === 'brute' ? 22 : 14));
    if (sourceGun?.payload === 'salvage') player.reserve += randomItem([2, 6, 14, 30]);
    if (sourceGun?.payload === 'siphon') player.health = Math.min(100, player.health + randomItem([2, 8, 20, 45]));
    if (player.combo % 3 === 0) showToast('STREAK x' + multiplier + ' // +' + points * multiplier);
    if (Math.random() < .24) pickups.push({ x: enemy.x, y: enemy.y, type: Math.random() < .48 ? 'health' : 'ammo' });
    if (sourceGun?.payload === 'volatile') {
      for (const nearby of [...enemies]) {
        if (Math.hypot(nearby.x - enemy.x, nearby.y - enemy.y) < 2.15) {
          nearby.hp -= randomItem([1, 2, 5, 12]);
          nearby.hit = .3;
          if (nearby.hp <= 0) destroyEnemy(nearby, null);
        }
      }
    }
    playTone(610 + Math.min(player.combo, 8) * 35, 0.09, 'triangle', 0.035);
    updateHud();
    if (!enemies.length && !waveResolving) { waveResolving = true; clearWave(); }
  }

  function ionSurge() {
    if (gameState !== 'playing' || player.surge < 100) return;
    player.surge = 0;
    const targets = [...enemies];
    for (const enemy of targets) {
      const distance = Math.hypot(enemy.x - player.x, enemy.y - player.y);
      if (distance > 7 || castRay(Math.atan2(enemy.y - player.y, enemy.x - player.x)).distance + .2 < distance) continue;
      enemy.hp -= enemy.kind === 'brute' ? 2 : 3;
      enemy.hit = .32;
      if (enemy.hp <= 0) destroyEnemy(enemy);
    }
    muzzleFlash = .75;
    playTone(180, .36, 'sawtooth', .07);
    showToast('ION SURGE // CLEAR THE SKY');
    updateHud();
  }

  function updateEnemies(dt) {
    for (const enemy of enemies) {
      const dx = player.x - enemy.x; const dy = player.y - enemy.y; const distance = Math.hypot(dx, dy);
      enemy.hit = Math.max(0, enemy.hit - dt); enemy.cooldown -= dt;
      enemy.phase += dt * 2.2;
      if (enemy.burnTimer > 0) {
        enemy.burnTimer -= dt; enemy.burnTick -= dt;
        if (enemy.burnTick <= 0) {
          enemy.burnTick = .35; enemy.hp -= enemy.burnDamage;
          if (enemy.hp <= 0) { destroyEnemy(enemy, null); continue; }
        }
      }
      if (enemy.frostTimer > 0) enemy.frostTimer = Math.max(0, enemy.frostTimer - dt);
      if (enemy.kind === 'skimmer' && distance < 6 && distance > 3) {
        const orbit = Math.sin(enemy.phase) * .16;
        const mx = (dx / distance * .35 - dy / distance * orbit) * dt;
        const my = (dy / distance * .35 + dx / distance * orbit) * dt;
        if (!isWall(enemy.x + mx, enemy.y)) enemy.x += mx;
        if (!isWall(enemy.x, enemy.y + my)) enemy.y += my;
        if (enemy.cooldown <= 0) {
          enemy.cooldown = 1.7 + Math.random() * .5;
          if (Math.random() < .55) {
            player.health = Math.max(0, player.health - 7); damageFlash = .8; showToast('SKIMMER VOLLEY // BREAK LINE'); updateHud();
            if (player.health <= 0) { finishRun(); break; }
          }
        }
      } else if (distance > (enemy.kind === 'skimmer' ? 3.2 : 1.05)) {
        const speed = enemy.speed * (enemy.frostTimer > 0 ? .4 : 1) * dt; const mx = dx / distance * speed; const my = dy / distance * speed;
        if (!isWall(enemy.x + mx + Math.sign(mx) * 0.16, enemy.y)) enemy.x += mx;
        if (!isWall(enemy.x, enemy.y + my + Math.sign(my) * 0.16)) enemy.y += my;
      } else if (enemy.cooldown <= 0) {
        enemy.cooldown = 1.15 + Math.random() * 0.55;
        player.health = Math.max(0, player.health - (enemy.kind === 'brute' ? 16 : 9));
        damageFlash = .8; showToast('IMPACT // KEEP MOVING'); playTone(120, 0.1, 'sawtooth', 0.04); updateHud();
        if (player.health <= 0) finishRun();
        break;
      }
    }
  }

  function update(dt) {
    if (gameState !== 'playing') return;
    worldTime += dt; shotCooldown = Math.max(0, shotCooldown - dt); muzzleFlash = Math.max(0, muzzleFlash - dt * 6); recoil = Math.max(0, recoil - dt * 7); damageFlash = Math.max(0, damageFlash - dt * 2.4);
    plasmaBursts = plasmaBursts.filter((burst) => (burst.life -= dt) > 0);
    if (player.comboTimer > 0 && (player.comboTimer -= dt) <= 0) { player.combo = 0; updateHud(); }
    if (toastTimer > 0 && (toastTimer -= dt) <= 0) ui.toast.classList.remove('show');
    if (reloadTime > 0 && (reloadTime -= dt) <= 0) {
      const amount = Math.min(selectedWeapon.magazine - player.ammo, player.reserve);
      player.ammo += amount; player.reserve -= amount; updateHud(); playTone(260, 0.07, 'triangle', 0.025);
      bonusNextShot = true;
    }
    let forward = Number(keys.has('KeyW') || keys.has('ArrowUp') || touchMoves.has('forward')) - Number(keys.has('KeyS') || keys.has('ArrowDown') || touchMoves.has('back'));
    let strafe = Number(keys.has('KeyD') || keys.has('ArrowRight') || touchMoves.has('right')) - Number(keys.has('KeyA') || keys.has('ArrowLeft') || touchMoves.has('left'));
    const length = Math.hypot(forward, strafe) || 1; forward /= length; strafe /= length;
    const speed = (keys.has('ShiftLeft') ? 3.25 : 2.45) * dt;
    movePlayer((Math.cos(player.angle) * forward - Math.sin(player.angle) * strafe) * speed,
      (Math.sin(player.angle) * forward + Math.cos(player.angle) * strafe) * speed);
    pickups = pickups.filter((pickup) => {
      if (Math.hypot(pickup.x - player.x, pickup.y - player.y) > .58) return true;
      if (pickup.type === 'health') { player.health = Math.min(100, player.health + 24); showToast('FIELD PATCH // +24 VITALS'); }
      else { player.reserve += 18; showToast('CHARGE CELL // +18 AMMO'); }
      updateHud(); playTone(520, .11, 'sine', .035); return false;
    });
    if (fireHeld) shoot();
    updateEnemies(dt);
  }

  function renderMenu() {
    const gradient = ctx.createLinearGradient(0, 0, width, height);
    gradient.addColorStop(0, '#152638'); gradient.addColorStop(0.53, '#282235'); gradient.addColorStop(1, '#0b1017');
    ctx.fillStyle = gradient; ctx.fillRect(0, 0, width, height);
    ctx.strokeStyle = 'rgba(112,240,223,.14)';
    for (let line = 0; line < 14; line += 1) {
      const y = height * 0.56 + line * line * 0.5;
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(width, y); ctx.stroke();
    }
    for (let line = -8; line <= 8; line += 1) {
      ctx.beginPath(); ctx.moveTo(width / 2 + line * 14, height); ctx.lineTo(width / 2 + line * width * 0.12, height * 0.54); ctx.stroke();
    }
  }

  function render() {
    if (!ctx || !width || !height) return;
    if (gameState === 'playing' || gameState === 'paused') drawWorld();
    else renderMenu();
  }

  function frame(time) {
    const dt = Math.min(0.04, (time - lastTime) / 1000 || 0);
    lastTime = time; update(dt); render(); requestAnimationFrame(frame);
  }

  $('#start-button').addEventListener('click', openArmory);
  $('#resume-button').addEventListener('click', () => { setState('playing'); requestPointerLock(); });
  $('#restart-button').addEventListener('click', openArmory);
  $('#play-again-button').addEventListener('click', openArmory);
  $('#sound-button').addEventListener('click', (event) => {
    soundEnabled = !soundEnabled;
    event.currentTarget.setAttribute('aria-pressed', String(soundEnabled));
    event.currentTarget.setAttribute('aria-label', soundEnabled ? 'Mute sound' : 'Enable sound');
  });
  $('#touch-medkit').addEventListener('click', medkit);
  $('#touch-surge').addEventListener('click', ionSurge);
  $('#touch-fire').addEventListener('pointerdown', (event) => { event.preventDefault(); $('#touch-fire').setPointerCapture(event.pointerId); fireHeld = true; });
  for (const name of ['pointerup', 'pointercancel', 'pointerleave']) $('#touch-fire').addEventListener(name, () => { fireHeld = false; });
  document.querySelectorAll('[data-move]').forEach((button) => {
    const direction = button.dataset.move;
    button.addEventListener('pointerdown', (event) => { event.preventDefault(); button.setPointerCapture(event.pointerId); touchMoves.add(direction); });
    for (const name of ['pointerup', 'pointercancel', 'pointerleave']) button.addEventListener(name, () => touchMoves.delete(direction));
  });
  canvas.addEventListener('pointerdown', (event) => {
    if (gameState !== 'playing') return;
    if (event.pointerType === 'touch') { touchAimX = event.clientX; return; }
    fireHeld = true; requestPointerLock();
  });
  canvas.addEventListener('pointermove', (event) => {
    if (touchAimX !== null && event.pointerType === 'touch') {
      player.angle += (event.clientX - touchAimX) * 0.006;
      touchAimX = event.clientX;
    }
  });
  canvas.addEventListener('pointerup', () => { touchAimX = null; });
  canvas.addEventListener('pointercancel', () => { touchAimX = null; });
  window.addEventListener('pointerup', () => { fireHeld = false; });
  document.addEventListener('mousemove', (event) => {
    if (gameState === 'playing' && document.pointerLockElement === canvas) player.angle += event.movementX * 0.0027;
  });
  document.addEventListener('pointerlockchange', () => {
    if (document.pointerLockElement !== canvas && gameState === 'playing' && !matchMedia('(pointer: coarse)').matches) setState('paused');
  });
  window.addEventListener('keydown', (event) => {
    if (['Space', 'ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(event.code)) event.preventDefault();
    keys.add(event.code);
    if (gameState === 'menu' && (event.code === 'Enter' || event.code === 'Space')) openArmory();
    if (gameState === 'armory' && event.code === 'Escape') setState('menu');
    if (event.code === 'Escape' && gameState === 'playing') setState('paused');
    if (event.code === 'Escape' && gameState === 'paused') { setState('playing'); requestPointerLock(); }
    if (event.code === 'KeyR') reload();
    if (event.code === 'KeyF' || event.code === 'KeyQ') medkit();
    if (event.code === 'KeyE') ionSurge();
    if (event.code === 'Space' && gameState === 'playing') fireHeld = true;
  });
  window.addEventListener('keyup', (event) => { keys.delete(event.code); if (event.code === 'Space') fireHeld = false; });
  window.addEventListener('blur', () => { keys.clear(); touchMoves.clear(); fireHeld = false; if (gameState === 'playing') setState('paused'); });
  document.addEventListener('visibilitychange', () => { if (document.hidden && gameState === 'playing') setState('paused'); });
  window.addEventListener('resize', resize);
  resize(); setState('menu'); requestAnimationFrame(frame);
})();

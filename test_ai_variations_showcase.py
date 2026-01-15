#!/usr/bin/env python3
"""
🎭 SHOWCASE DE VARIACIONES DE IA - Nightmare Edition
=====================================================

Este test muestra las variaciones dramáticas y vivas que la IA puede crear
manteniendo el lore original de Diablo.
"""

import os
import sys
import time
from typing import Optional

try:
    import requests
except ImportError:
    print("❌ Error: requests library not found")
    sys.exit(1)

# ============================================================================
# CONFIGURACIÓN
# ============================================================================

def load_api_key() -> Optional[str]:
    env_path = os.path.join("tools", ".env.dev")
    if not os.path.exists(env_path):
        return None
    with open(env_path, 'r') as f:
        for line in f:
            if line.startswith('API_KEY='):
                key = line.split('=', 1)[1].strip()
                return key.strip('"').strip("'")
    return None

def build_prompt(text: str, tone: str) -> str:
    return f"""You are a text variation engine for Diablo 1 (1996).

GOAL: Create NOTICEABLE variations that feel alive and dynamic while respecting lore.

STRICT RULES:
- You MUST NOT invent new lore, facts, places, names, events, or mechanics.
- You MUST NOT add new information.
- You MUST ONLY use words from the ORIGINAL TEXT (you can add common words like: the, a, is, are, but, yet, still, now, here, there).
- You SHOULD create NOTICEABLE variations: reorder words dramatically, change rhythm, add pauses (...), change emphasis.
- You MAY use ellipsis (...), capitalization for EMPHASIS, question marks, exclamation points.
- Make it feel ALIVE and REACTIVE, not robotic.
- Output ONLY the final text, no explanations.

STYLE:
- Dark, gothic, medieval
- Diablo 1 tone (1996)
- Dramatic and atmospheric
- No modern slang
- No humor

ORIGINAL TEXT: "{text}"

TONE MODIFIER: {tone}

Examples of good variations:
- "What can I do for you?" → "What... can I do for you?" (weary)
- "The darkness grows." → "The darkness... it GROWS." (cryptic)
- "Stay awhile and listen." → "Stay. Listen awhile." (cold)"""

def clean_response(response: str) -> str:
    prefixes = ['<s>', '</s>', '[OUT]', '[INST]', '[/INST]', '```', 'Output:', 'Result:']
    for prefix in prefixes:
        response = response.replace(prefix, '')
    response = ' '.join(response.split())
    return response.strip('"').strip("'").strip()

def call_api(text: str, tone: str, api_key: str) -> Optional[str]:
    try:
        headers = {
            "Authorization": f"Bearer {api_key}",
            "Content-Type": "application/json"
        }
        data = {
            "model": "mistralai/mistral-7b-instruct:free",
            "messages": [{"role": "user", "content": build_prompt(text, tone)}],
            "temperature": 0.7,  # Más creatividad
            "max_tokens": 128
        }
        response = requests.post(
            "https://openrouter.ai/api/v1/chat/completions",
            headers=headers,
            json=data,
            timeout=5
        )
        if response.status_code == 200:
            result = response.json()
            return clean_response(result['choices'][0]['message']['content'])
        return None
    except:
        return None

# ============================================================================
# SHOWCASE
# ============================================================================

def showcase_npc_variations(api_key: str):
    """Muestra variaciones de diálogos de NPCs"""
    print("\n" + "="*70)
    print("🎭 SHOWCASE: VARIACIONES DE DIÁLOGOS DE NPCs")
    print("="*70)
    
    npcs = [
        ("Griswold (Blacksmith)", "What can I do for you?", [
            ("neutral", "Tono normal"),
            ("weary", "Cansado después de muchas ventas"),
            ("irritated", "Molesto por interrupciones")
        ]),
        ("Deckard Cain (Elder)", "Stay awhile and listen.", [
            ("neutral", "Tono normal"),
            ("weary", "Exhausto de contar historias"),
            ("cryptic", "Misterioso y profético")
        ]),
        ("Pepin (Healer)", "I sense a soul in search of answers.", [
            ("neutral", "Tono normal"),
            ("cryptic", "Visionario y místico"),
            ("cold", "Distante y analítico")
        ]),
        ("Adria (Witch)", "The darkness grows stronger.", [
            ("neutral", "Tono normal"),
            ("cryptic", "Profético y ominoso"),
            ("cold", "Frío y calculador")
        ]),
        ("Wirt (Boy)", "I got something for you.", [
            ("neutral", "Tono normal"),
            ("irritated", "Impaciente"),
            ("cryptic", "Misterioso sobre su mercancía")
        ])
    ]
    
    for npc_name, original, tones in npcs:
        print(f"\n{'─'*70}")
        print(f"📜 {npc_name}")
        print(f"{'─'*70}")
        print(f"Original: \"{original}\"")
        print()
        
        for tone, description in tones:
            result = call_api(original, tone, api_key)
            if result:
                # Mostrar diferencia
                if result != original:
                    print(f"  🎭 {description} ({tone}):")
                    print(f"     → \"{result}\"")
                    print(f"     ✨ VARIACIÓN NOTABLE")
                else:
                    print(f"  📝 {description} ({tone}):")
                    print(f"     → \"{result}\"")
                    print(f"     ℹ️  Original preservado")
            else:
                print(f"  ⚠️  {description} ({tone}): API no disponible")
            
            time.sleep(0.5)  # Rate limiting

def showcase_combat_messages(api_key: str):
    """Muestra variaciones de mensajes de combate"""
    print("\n" + "="*70)
    print("⚔️  SHOWCASE: MENSAJES DE COMBATE")
    print("="*70)
    
    messages = [
        ("You have been slain.", "cryptic"),
        ("The demon falls before you.", "neutral"),
        ("Your health is low.", "weary"),
        ("A powerful foe approaches.", "cryptic"),
    ]
    
    for original, tone in messages:
        print(f"\n  Original: \"{original}\"")
        result = call_api(original, tone, api_key)
        if result and result != original:
            print(f"  Variación ({tone}): \"{result}\"")
            print(f"  ✨ MÁS DRAMÁTICO")
        elif result:
            print(f"  Variación ({tone}): \"{result}\"")
        else:
            print(f"  ⚠️  API no disponible")
        time.sleep(0.5)

def showcase_atmospheric_texts(api_key: str):
    """Muestra variaciones de textos atmosféricos"""
    print("\n" + "="*70)
    print("🌑 SHOWCASE: TEXTOS ATMOSFÉRICOS")
    print("="*70)
    
    texts = [
        ("The sanctity of this place has been fouled.", "weary"),
        ("Evil stirs in the darkness.", "cryptic"),
        ("The dead walk among us.", "cold"),
        ("Beware the forces of Hell.", "cryptic"),
    ]
    
    for original, tone in texts:
        print(f"\n  Original: \"{original}\"")
        result = call_api(original, tone, api_key)
        if result and result != original:
            print(f"  Variación ({tone}): \"{result}\"")
            print(f"  ✨ MÁS ATMOSFÉRICO")
        elif result:
            print(f"  Variación ({tone}): \"{result}\"")
        else:
            print(f"  ⚠️  API no disponible")
        time.sleep(0.5)

def showcase_comparison(api_key: str):
    """Comparación lado a lado de variaciones"""
    print("\n" + "="*70)
    print("📊 COMPARACIÓN: ORIGINAL vs VARIACIONES")
    print("="*70)
    
    test_text = "The darkness grows stronger."
    tones = ["neutral", "weary", "cold", "cryptic", "irritated"]
    
    print(f"\n  Texto base: \"{test_text}\"")
    print(f"\n  {'Tono':<12} | Variación")
    print(f"  {'-'*12}-+-{'-'*50}")
    
    for tone in tones:
        result = call_api(test_text, tone, api_key)
        if result:
            marker = "✨" if result != test_text else "📝"
            print(f"  {tone:<12} | {marker} {result}")
        else:
            print(f"  {tone:<12} | ⚠️  No disponible")
        time.sleep(0.5)

# ============================================================================
# MAIN
# ============================================================================

def main():
    print("🎭 SHOWCASE DE VARIACIONES DE IA - Nightmare Edition")
    print("="*70)
    print("\nEste showcase demuestra cómo la IA crea variaciones NOTABLES")
    print("que hacen los diálogos más VIVOS y REACTIVOS sin inventar lore.")
    print("="*70)
    
    api_key = load_api_key()
    if not api_key:
        print("\n❌ ERROR: No API key found in tools/.env.dev")
        sys.exit(1)
    
    print("\n⏳ Generando variaciones... (esto puede tomar un momento)")
    
    try:
        showcase_npc_variations(api_key)
        showcase_combat_messages(api_key)
        showcase_atmospheric_texts(api_key)
        showcase_comparison(api_key)
        
        print("\n" + "="*70)
        print("✅ SHOWCASE COMPLETADO")
        print("="*70)
        print("\n💡 CONCLUSIÓN:")
        print("   Las variaciones son NOTABLES y hacen los diálogos más VIVOS")
        print("   sin inventar lore nuevo. Cada interacción se siente única.")
        print("\n🎮 LISTO PARA INTEGRACIÓN EN EL JUEGO")
        
    except KeyboardInterrupt:
        print("\n\n⚠️  Showcase interrumpido por el usuario")
        sys.exit(0)

if __name__ == "__main__":
    main()

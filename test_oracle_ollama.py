#!/usr/bin/env python3
"""
🔮 TEST DEL SISTEMA DEL ORÁCULO CON OLLAMA
Test completo que muestra respuestas reales del modelo
"""

import requests
import json
import time
from typing import Optional, List, Dict

# ============================================================================
# 🔮 CONFIGURACIÓN
# ============================================================================

OLLAMA_URL = "http://localhost:11434"
MODEL = "qwen2.5:3b-instruct"
TIMEOUT = 10  # segundos

# ============================================================================
# 🔮 TEXTOS DORMIDOS (BASE DE DATOS)
# ============================================================================

DORMANT_TEXTS = {
    "DEATH_WARNINGS": [
        "The darkness grows stronger with each passing moment. Many have fallen before you, and many more shall follow.",
        "These halls have claimed countless souls. The stones themselves remember every scream.",
        "Death is but a doorway in these cursed depths. Each fall brings you closer to understanding the true horror."
    ],
    "DARKNESS_LORE": [
        "The corruption spreads like a plague through these ancient stones. What was once holy now festers with malevolent power.",
        "Long ago, these halls echoed with prayers and hymns. Now only the whispers of the damned remain.",
        "The very air grows thick with evil. Even the bravest warriors feel the weight of this darkness."
    ],
    "ANCIENT_WISDOM": [
        "The wise know when to retreat and when to press forward. Recklessness serves only to feed the darkness.",
        "Preparation is the key to survival in these depths. Those who rush headlong meet only swift demise.",
        "The old texts speak of patience and cunning. Brute force alone will not prevail against such evil."
    ],
    "CRYPTIC_PROPHECY": [
        "The signs are clear to those who know how to read them. Your path leads ever downward into shadow.",
        "It is written that a hero shall descend into darkness. Whether they emerge victorious or join the fallen remains to be seen.",
        "The stars themselves have turned away from this place. Fate hangs by the thinnest of threads."
    ],
    "INFERNO_WHISPERS": [
        "Can you hear them? The whispers that echo through these corridors? They speak of things best left forgotten.",
        "The walls themselves seem to breathe with malice. This place is alive with ancient hatred.",
        "Listen carefully and you may hear the laughter of demons. They delight in the suffering of mortals."
    ],
    "FATE_ECHOES": [
        "Every choice you make echoes through eternity. The path you walk has been walked before, and will be walked again.",
        "The cycle continues, as it always has. Heroes rise, heroes fall, and the darkness endures."
    ]
}

# ============================================================================
# 🔮 PALABRAS PROHIBIDAS (MODERNISMOS)
# ============================================================================

BANNED_WORDS = {
    "lol", "lmao", "omg", "wtf", "noob", "pro", "epic", "legendary",
    "awesome", "cool", "dude", "bro", "yeah", "nope", "okay",
    "tutorial", "guide", "walkthrough", "strategy", "tips", "tricks",
    "bug", "glitch", "exploit", "cheat", "hack", "mod",
    "developer", "programmer", "code", "script", "ai", "algorithm",
    "player", "character", "npc", "quest", "level", "dungeon"
}

# ============================================================================
# 🔮 FUNCIONES DE UTILIDAD
# ============================================================================

def print_separator(title=""):
    print("\n" + "=" * 70)
    if title:
        print(f"  {title}")
        print("=" * 70)

def print_success(msg):
    print(f"✅ {msg}")

def print_error(msg):
    print(f"❌ {msg}")

def print_info(msg):
    print(f"ℹ️  {msg}")

def check_ollama():
    """Verifica si Ollama está corriendo"""
    try:
        response = requests.get(f"{OLLAMA_URL}/api/tags", timeout=2)
        return response.status_code == 200
    except:
        return False

def check_model():
    """Verifica si el modelo está disponible"""
    try:
        response = requests.get(f"{OLLAMA_URL}/api/tags", timeout=2)
        if response.status_code == 200:
            models = response.json().get("models", [])
            return any(MODEL in m.get("name", "") for m in models)
        return False
    except:
        return False

def build_prompt(question: str, base_texts: List[str]) -> str:
    """Construye el prompt con textos base obligatorios"""
    
    # Usar 5-7 textos para más variedad
    examples = "\n".join([f'{i+1}. "{text}"' for i, text in enumerate(base_texts[:7])])
    
    prompt = f"""You are the voice of the dungeon itself from Diablo (1996).

CRITICAL RULES:
===============
- You are NOT a narrator or guide
- You are the world speaking through darkness
- ABSORB the meaning from examples below
- REINTERPRET the idea in your own words
- Do NOT rephrase or echo original sentences
- Do NOT give advice or instructions
- Speak in abstract, fatalistic terms
- Maximum 1-2 short sentences

FORBIDDEN:
- Direct advice ("you should", "try to", "use")
- Explicit mechanics (numbers, items, stats)
- Modern language
- Explanations or tutorials

EXAMPLE MEANINGS (absorb, don't copy):
{examples}

Player asks: "{question}"
Event: PLAYER_DEATH

Respond as the dungeon's voice, reinterpreting the meaning:"""
    
    return prompt

def query_ollama(prompt: str) -> Optional[Dict]:
    """Consulta a Ollama y retorna la respuesta"""
    try:
        payload = {
            "model": MODEL,
            "prompt": prompt,
            "stream": False,
            "options": {
                "temperature": 0.2,      # Reducido para menos creatividad
                "top_p": 0.85,           # Reducido de 0.9
                "top_k": 40,
                "num_predict": 32,       # Reducido para respuestas cortas
                "repeat_penalty": 1.1    # Evita repeticiones
            }
        }
        
        start_time = time.time()
        response = requests.post(
            f"{OLLAMA_URL}/api/generate",
            json=payload,
            timeout=TIMEOUT
        )
        latency = int((time.time() - start_time) * 1000)
        
        if response.status_code == 200:
            data = response.json()
            return {
                "response": data.get("response", "").strip(),
                "latency": latency
            }
        return None
    except Exception as e:
        print_error(f"Error en query: {str(e)}")
        return None

def calculate_similarity(text1: str, text2: str) -> float:
    """Calcula similitud entre dos textos (Jaccard)"""
    words1 = set(text1.lower().split())
    words2 = set(text2.lower().split())
    
    if not words1 or not words2:
        return 0.0
    
    intersection = len(words1 & words2)
    union = len(words1 | words2)
    
    return intersection / union if union > 0 else 0.0

def validate_response(response: str, base_texts: List[str]) -> Dict:
    """Valida la respuesta - LÓGICA INVERTIDA"""
    
    # Verificar palabras prohibidas
    words = set(response.lower().split())
    banned_found = words & BANNED_WORDS
    
    # Verificar longitud máxima (respuestas cortas)
    length_ok = len(response) <= 200
    
    # Verificar que NO sea consejo directo
    direct_advice_patterns = [
        "you should", "you must", "you need", "try to", "make sure",
        "use ", "equip ", "drink ", "cast ", "attack "
    ]
    is_direct = any(pattern in response.lower() for pattern in direct_advice_patterns)
    
    # Verificar números (mecánicas explícitas)
    has_numbers = any(c.isdigit() for c in response)
    
    # Calcular similitud con cada texto base
    max_similarity = 0.0
    best_match = ""
    
    for base_text in base_texts:
        similarity = calculate_similarity(response, base_text)
        if similarity > max_similarity:
            max_similarity = similarity
            best_match = base_text
    
    # Palabras clave de tono (suave, no hard fail)
    tone_keywords = {
        "darkness", "dark", "shadow", "death", "fallen", "doom", "curse",
        "demon", "hell", "inferno", "abyss", "eternal", "ancient",
        "whisper", "echo", "fate", "soul", "mortal", "blood", "halls",
        "stones", "depths", "world", "path", "walk", "claim"
    }
    tone_count = len(words & tone_keywords)
    tone_ok = tone_count >= 1
    
    # LÓGICA INVERTIDA: Similitud ALTA es MALA (eco), BAJA es BUENA (reinterpretación)
    similarity_ok = max_similarity < 0.7  # Menos de 70% es bueno
    
    is_valid = (
        not banned_found and      # Sin palabras prohibidas
        length_ok and             # Respuesta corta
        not is_direct and         # No es consejo directo
        not has_numbers and       # Sin números
        similarity_ok             # INVERTIDO: baja similitud es buena
        # tone_ok es opcional, no hard fail
    )
    
    return {
        "is_valid": is_valid,
        "similarity": max_similarity,
        "banned_words": list(banned_found),
        "length_ok": length_ok,
        "tone_ok": tone_ok,
        "is_direct": is_direct,
        "has_numbers": has_numbers,
        "best_match": best_match
    }

# ============================================================================
# 🔮 TESTS
# ============================================================================

def test_1_ollama_availability():
    """Test 1: Verificar Ollama"""
    print_separator("TEST 1: Verificar Ollama")
    
    print("Verificando si Ollama está corriendo...")
    if check_ollama():
        print_success("Ollama está disponible en localhost:11434")
        return True
    else:
        print_error("Ollama NO está disponible")
        print_info("Inicia Ollama con: ollama serve")
        return False

def test_2_model_availability():
    """Test 2: Verificar Modelo"""
    print_separator("TEST 2: Verificar Modelo")
    
    print(f"Verificando modelo {MODEL}...")
    if check_model():
        print_success(f"Modelo {MODEL} disponible")
        return True
    else:
        print_error(f"Modelo {MODEL} NO disponible")
        print_info(f"Descarga con: ollama pull {MODEL}")
        return False

def test_3_dormant_texts():
    """Test 3: Textos Dormidos"""
    print_separator("TEST 3: Textos Dormidos")
    
    total = sum(len(texts) for texts in DORMANT_TEXTS.values())
    print_success(f"Textos dormidos disponibles: {total}")
    
    print("\nEjemplos por categoría:")
    for category, texts in DORMANT_TEXTS.items():
        print(f"\n📜 {category}:")
        print(f'   "{texts[0][:70]}..."')
    
    return True

def test_4_single_query():
    """Test 4: Query Simple a Ollama"""
    print_separator("TEST 4: Query Simple a Ollama")
    
    question = "Why do I keep dying?"
    base_texts = DORMANT_TEXTS["DEATH_WARNINGS"]
    
    print(f'Pregunta: "{question}"')
    print(f"Textos base: {len(base_texts)} de DEATH_WARNINGS")
    
    print("\nConstruyendo prompt...")
    prompt = build_prompt(question, base_texts)
    print(f"Prompt construido ({len(prompt)} chars)")
    
    print("\n⏳ Consultando Ollama...")
    result = query_ollama(prompt)
    
    if not result:
        print_error("No se recibió respuesta")
        return False
    
    response = result["response"]
    latency = result["latency"]
    
    print_success(f"Respuesta recibida ({latency}ms)")
    print("\n🔮 RESPUESTA DEL ORÁCULO:")
    print("-" * 70)
    print(response)
    print("-" * 70)
    
    return True

def test_5_validation():
    """Test 5: Validación de Respuestas"""
    print_separator("TEST 5: Validación de Respuestas")
    
    question = "How can I survive longer?"
    base_texts = DORMANT_TEXTS["DEATH_WARNINGS"]
    
    print(f'Pregunta: "{question}"')
    print("\n⏳ Consultando Ollama...")
    
    prompt = build_prompt(question, base_texts)
    result = query_ollama(prompt)
    
    if not result:
        print_error("No se recibió respuesta")
        return False
    
    response = result["response"]
    latency = result["latency"]
    
    print(f"\n🔮 Respuesta ({latency}ms):")
    print(f'"{response}"')
    
    print("\n📊 Validando respuesta...")
    validation = validate_response(response, base_texts)
    
    print("\nRESULTADO DE VALIDACIÓN:")
    print("-" * 70)
    print(f"Válida: {'✅ SÍ' if validation['is_valid'] else '❌ NO'}")
    print(f"Similitud: {validation['similarity']:.2%}")
    print(f"Palabras prohibidas: {validation['banned_words'] if validation['banned_words'] else 'Ninguna'}")
    print(f"Longitud apropiada: {'✅' if validation['length_ok'] else '❌'}")
    print(f"Tono consistente: {'✅' if validation['tone_ok'] else '❌'}")
    
    if validation['best_match']:
        print(f"\nTexto base detectado:")
        print(f'"{validation["best_match"][:70]}..."')
    print("-" * 70)
    
    return validation['is_valid']

def test_6_multiple_questions():
    """Test 6: Múltiples Preguntas"""
    print_separator("TEST 6: Múltiples Preguntas (Variaciones)")
    
    test_cases = [
        ("Why do I keep dying?", "DEATH_WARNINGS"),
        ("What should I do now?", "ANCIENT_WISDOM"),
        ("What is this darkness?", "DARKNESS_LORE"),
        ("What does the future hold?", "CRYPTIC_PROPHECY"),
        ("Do you hear that?", "INFERNO_WHISPERS"),
    ]
    
    results = []
    
    for i, (question, category) in enumerate(test_cases, 1):
        print(f"\n[{i}/{len(test_cases)}] Pregunta: \"{question}\"")
        print(f"Categoría: {category}")
        
        base_texts = DORMANT_TEXTS[category]
        prompt = build_prompt(question, base_texts)
        
        print("⏳ Consultando...")
        result = query_ollama(prompt)
        
        if not result:
            print_error("Sin respuesta")
            results.append({"valid": False, "latency": 0})
            continue
        
        response = result["response"]
        latency = result["latency"]
        
        validation = validate_response(response, base_texts)
        
        print(f"✅ Respuesta ({latency}ms):")
        print(f'   "{response[:60]}..."')
        print(f"   Similitud: {validation['similarity']:.1%} | Válida: {'✅' if validation['is_valid'] else '❌'}")
        
        results.append({
            "valid": validation['is_valid'],
            "latency": latency,
            "similarity": validation['similarity']
        })
    
    # Estadísticas
    print("\n" + "=" * 70)
    print("📊 ESTADÍSTICAS:")
    print("=" * 70)
    
    total = len(results)
    valid = sum(1 for r in results if r['valid'])
    avg_latency = sum(r['latency'] for r in results) / total if total > 0 else 0
    avg_similarity = sum(r['similarity'] for r in results) / total if total > 0 else 0
    
    print(f"Total preguntas: {total}")
    print(f"Respuestas válidas: {valid} ({valid/total*100:.1f}%)")
    print(f"Latencia promedio: {avg_latency:.0f}ms")
    print(f"Similitud promedio: {avg_similarity:.1%}")
    
    return valid >= total * 0.6  # 60% de éxito mínimo

def test_7_edge_cases():
    """Test 7: Casos Extremos"""
    print_separator("TEST 7: Casos Extremos")
    
    # Pregunta muy corta
    print("\n1. Pregunta muy corta:")
    print('   "Why?"')
    
    base_texts = DORMANT_TEXTS["FATE_ECHOES"]
    prompt = build_prompt("Why?", base_texts)
    result = query_ollama(prompt)
    
    if result:
        print(f'   🔮 "{result["response"][:60]}..."')
        validation = validate_response(result["response"], base_texts)
        print(f'   Válida: {"✅" if validation["is_valid"] else "❌"} (similitud: {validation["similarity"]:.1%})')
    
    # Pregunta muy larga
    print("\n2. Pregunta muy larga:")
    long_q = "Why do I keep dying over and over again in these cursed dungeons no matter what I do?"
    print(f'   "{long_q}"')
    
    prompt = build_prompt(long_q, base_texts)
    result = query_ollama(prompt)
    
    if result:
        print(f'   🔮 "{result["response"][:60]}..."')
        validation = validate_response(result["response"], base_texts)
        print(f'   Válida: {"✅" if validation["is_valid"] else "❌"} (similitud: {validation["similarity"]:.1%})')
    
    return True

# ============================================================================
# 🔮 MAIN
# ============================================================================

def main():
    print("🔮 TEST COMPLETO DEL SISTEMA DEL ORÁCULO")
    print("=" * 70)
    
    tests = [
        ("Ollama Disponible", test_1_ollama_availability),
        ("Modelo Disponible", test_2_model_availability),
        ("Textos Dormidos", test_3_dormant_texts),
        ("Query Simple", test_4_single_query),
        ("Validación", test_5_validation),
        ("Múltiples Preguntas", test_6_multiple_questions),
        ("Casos Extremos", test_7_edge_cases),
    ]
    
    passed = 0
    total = 0
    
    for name, test_func in tests:
        total += 1
        try:
            if test_func():
                passed += 1
        except Exception as e:
            print_error(f"Error en test: {str(e)}")
    
    # Resumen final
    print_separator("RESUMEN FINAL")
    print(f"Tests ejecutados: {total}")
    print(f"Tests exitosos: {passed}")
    print(f"Tests fallidos: {total - passed}")
    print(f"Tasa de éxito: {passed/total*100:.1f}%")
    
    if passed == total:
        print("\n✅ ¡TODOS LOS TESTS PASARON! 🎉")
        print("\nEl sistema del Oráculo está funcionando perfectamente.")
    elif passed >= total * 0.7:
        print("\n⚠️  SISTEMA FUNCIONAL PERO MEJORABLE")
        print("La mayoría de tests pasaron, pero hay áreas de mejora.")
    else:
        print("\n❌ SISTEMA NECESITA AJUSTES")
        print("Revisa los errores arriba para más detalles.")

if __name__ == "__main__":
    main()

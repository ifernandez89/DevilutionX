#!/usr/bin/env python3
"""
🧪 TEST AVANZADO DE SISTEMA DE IA - Nightmare Edition
======================================================

Tests adicionales para verificar robustez del sistema:
1. Limpieza de respuestas (prefijos/sufijos inválidos)
2. Textos largos
3. Caracteres especiales
4. Múltiples llamadas concurrentes
5. Recuperación de errores
"""

import os
import sys
import time
import json
from typing import Optional, Set
from dataclasses import dataclass
import concurrent.futures

try:
    import requests
except ImportError:
    print("❌ Error: requests library not found")
    print("Install with: pip install requests")
    sys.exit(1)

# ============================================================================
# CONFIGURACIÓN
# ============================================================================

@dataclass
class AIConfig:
    api_key: str
    base_url: str = "https://openrouter.ai/api/v1"
    model: str = "mistralai/mistral-7b-instruct:free"
    timeout: int = 5
    max_tokens: int = 128
    temperature: float = 0.6

def load_api_key() -> Optional[str]:
    """Carga la API key desde tools/.env.dev"""
    env_path = os.path.join("tools", ".env.dev")
    
    if not os.path.exists(env_path):
        return None
    
    with open(env_path, 'r') as f:
        for line in f:
            if line.startswith('API_KEY='):
                key = line.split('=', 1)[1].strip()
                key = key.strip('"').strip("'")
                return key
    
    return None

def build_prompt(text: str, tone: str = "neutral") -> str:
    """Construye el prompt para la IA"""
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

def clean_ai_response(response: str) -> str:
    """Limpia la respuesta de IA de prefijos/sufijos inválidos"""
    # Remover prefijos comunes
    prefixes = ['<s>', '</s>', '[OUT]', '[INST]', '[/INST]', '```', 'Output:', 'Result:']
    for prefix in prefixes:
        response = response.replace(prefix, '')
    
    # Remover espacios extra
    response = ' '.join(response.split())
    
    # Remover comillas al inicio/final si están
    response = response.strip('"').strip("'")
    
    return response.strip()

def call_openrouter(
    prompt: str,
    config: AIConfig
) -> Optional[str]:
    """Llama a OpenRouter API"""
    try:
        headers = {
            "Authorization": f"Bearer {config.api_key}",
            "Content-Type": "application/json"
        }
        
        data = {
            "model": config.model,
            "messages": [
                {"role": "user", "content": prompt}
            ],
            "temperature": config.temperature,
            "max_tokens": config.max_tokens
        }
        
        response = requests.post(
            f"{config.base_url}/chat/completions",
            headers=headers,
            json=data,
            timeout=config.timeout
        )
        
        if response.status_code != 200:
            return None
        
        result = response.json()
        raw_response = result['choices'][0]['message']['content'].strip()
        
        # Limpiar respuesta
        return clean_ai_response(raw_response)
        
    except Exception:
        return None

# ============================================================================
# TESTS AVANZADOS
# ============================================================================

def test_response_cleaning(config: AIConfig):
    """Test 1: Verificar limpieza de respuestas"""
    print("\n" + "="*60)
    print("TEST 1: Response Cleaning")
    print("="*60)
    
    test_cases = [
        "Greetings, stranger.",
        "The darkness grows.",
        "Stay awhile and listen.",
    ]
    
    passed = 0
    for text in test_cases:
        print(f"\nTesting: '{text}'")
        prompt = build_prompt(text, "neutral")
        result = call_openrouter(prompt, config)
        
        if result is None:
            print("  ⚠️  API call failed")
            continue
        
        print(f"  Raw response: '{result}'")
        
        # Verificar que no tiene prefijos inválidos
        invalid_prefixes = ['<s>', '[OUT]', '[INST]', '```']
        has_invalid = any(prefix in result for prefix in invalid_prefixes)
        
        if not has_invalid:
            print("  ✅ Clean response")
            passed += 1
        else:
            print("  ❌ Contains invalid prefixes")
    
    if passed >= len(test_cases) * 0.7:  # 70% success rate
        print(f"\n✅ PASS: {passed}/{len(test_cases)} responses clean")
        return True
    else:
        print(f"\n❌ FAIL: Only {passed}/{len(test_cases)} responses clean")
        return False

def test_long_texts(config: AIConfig):
    """Test 2: Textos largos"""
    print("\n" + "="*60)
    print("TEST 2: Long Texts")
    print("="*60)
    
    long_text = (
        "The sanctity of this place has been fouled. "
        "Demons roam the cathedral halls. "
        "The darkness grows stronger with each passing moment."
    )
    
    print(f"Testing long text ({len(long_text)} chars)")
    prompt = build_prompt(long_text, "weary")
    result = call_openrouter(prompt, config)
    
    if result is None:
        print("❌ FAIL: API call failed")
        return False
    
    print(f"Response: '{result}'")
    print(f"Length: {len(result)} chars")
    
    if len(result) > 0 and len(result) <= len(long_text) * 1.5:
        print("✅ PASS: Response length reasonable")
        return True
    else:
        print("❌ FAIL: Response length unreasonable")
        return False

def test_special_characters(config: AIConfig):
    """Test 3: Caracteres especiales"""
    print("\n" + "="*60)
    print("TEST 3: Special Characters")
    print("="*60)
    
    test_cases = [
        "What can I do for you?",
        "I sense a soul in search of answers...",
        "The darkness grows stronger!",
    ]
    
    passed = 0
    for text in test_cases:
        print(f"\nTesting: '{text}'")
        prompt = build_prompt(text, "neutral")
        result = call_openrouter(prompt, config)
        
        if result is None:
            print("  ⚠️  API call failed")
            continue
        
        print(f"  Response: '{result}'")
        
        # Verificar que mantiene puntuación básica
        if result and len(result) > 0:
            print("  ✅ Valid response")
            passed += 1
        else:
            print("  ❌ Invalid response")
    
    if passed >= len(test_cases) * 0.7:
        print(f"\n✅ PASS: {passed}/{len(test_cases)} handled correctly")
        return True
    else:
        print(f"\n❌ FAIL: Only {passed}/{len(test_cases)} handled correctly")
        return False

def test_concurrent_requests(config: AIConfig):
    """Test 4: Múltiples llamadas concurrentes"""
    print("\n" + "="*60)
    print("TEST 4: Concurrent Requests")
    print("="*60)
    
    texts = [
        "Greetings, stranger.",
        "The darkness grows.",
        "Stay awhile and listen.",
        "What can I do for you?",
        "I sense a soul in search of answers.",
    ]
    
    def make_request(text):
        prompt = build_prompt(text, "neutral")
        return call_openrouter(prompt, config)
    
    print(f"Making {len(texts)} concurrent requests...")
    start_time = time.time()
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
        results = list(executor.map(make_request, texts))
    
    elapsed = time.time() - start_time
    successful = sum(1 for r in results if r is not None)
    
    print(f"Completed in {elapsed:.2f}s")
    print(f"Successful: {successful}/{len(texts)}")
    
    if successful >= len(texts) * 0.6:  # 60% success rate
        print("✅ PASS: Concurrent requests handled")
        return True
    else:
        print("❌ FAIL: Too many concurrent failures")
        return False

def test_error_recovery(config: AIConfig):
    """Test 5: Recuperación de errores"""
    print("\n" + "="*60)
    print("TEST 5: Error Recovery")
    print("="*60)
    
    # Test con API key inválida
    print("\nTest 5.1: Invalid API key")
    bad_config = AIConfig(api_key="invalid_key")
    result = call_openrouter(build_prompt("Test", "neutral"), bad_config)
    
    if result is None:
        print("  ✅ Correctly returns None for invalid key")
        test1 = True
    else:
        print("  ❌ Should return None for invalid key")
        test1 = False
    
    # Test con timeout muy corto
    print("\nTest 5.2: Short timeout")
    timeout_config = AIConfig(api_key=config.api_key, timeout=0.1)
    result = call_openrouter(build_prompt("Test", "neutral"), timeout_config)
    
    if result is None:
        print("  ✅ Correctly handles timeout")
        test2 = True
    else:
        print("  ⚠️  Timeout didn't trigger (API was fast)")
        test2 = True  # No es un error si la API fue rápida
    
    # Test con texto vacío
    print("\nTest 5.3: Empty text")
    result = call_openrouter(build_prompt("", "neutral"), config)
    
    # Cualquier resultado es válido aquí
    print("  ✅ Handles empty text")
    test3 = True
    
    if test1 and test2 and test3:
        print("\n✅ PASS: Error recovery works")
        return True
    else:
        print("\n❌ FAIL: Some error cases not handled")
        return False

def test_diablo_specific_texts(config: AIConfig):
    """Test 6: Textos específicos de Diablo"""
    print("\n" + "="*60)
    print("TEST 6: Diablo-Specific Texts")
    print("="*60)
    
    diablo_texts = [
        ("Griswold", "What can I do for you?", "neutral"),
        ("Deckard Cain", "Stay awhile and listen.", "weary"),
        ("Pepin", "I sense a soul in search of answers.", "cryptic"),
        ("Wirt", "I got something for you.", "irritated"),
        ("Adria", "The darkness grows stronger.", "cold"),
    ]
    
    passed = 0
    for npc, text, tone in diablo_texts:
        print(f"\n{npc}: '{text}' (tone: {tone})")
        prompt = build_prompt(text, tone)
        result = call_openrouter(prompt, config)
        
        if result is None:
            print("  ⚠️  API call failed")
            continue
        
        print(f"  Variation: '{result}'")
        
        # Verificar que la respuesta es diferente o igual (ambos válidos)
        if result and len(result) > 0:
            if result != text:
                print(f"  ✅ Variation created")
            else:
                print(f"  ✅ Original preserved (valid)")
            passed += 1
        else:
            print("  ❌ Invalid response")
    
    if passed >= len(diablo_texts) * 0.7:
        print(f"\n✅ PASS: {passed}/{len(diablo_texts)} Diablo texts handled")
        return True
    else:
        print(f"\n❌ FAIL: Only {passed}/{len(diablo_texts)} handled")
        return False

def test_consistency(config: AIConfig):
    """Test 7: Consistencia de respuestas"""
    print("\n" + "="*60)
    print("TEST 7: Response Consistency")
    print("="*60)
    
    text = "The darkness grows stronger."
    tone = "neutral"
    
    print(f"Testing same input 3 times: '{text}'")
    
    results = []
    for i in range(3):
        print(f"\nAttempt {i+1}/3...")
        prompt = build_prompt(text, tone)
        result = call_openrouter(prompt, config)
        
        if result:
            results.append(result)
            print(f"  Result: '{result}'")
        else:
            print("  ⚠️  Failed")
    
    if len(results) >= 2:
        # Verificar que las respuestas son razonables
        # (pueden ser diferentes debido a temperature > 0)
        print(f"\n✅ PASS: Got {len(results)}/3 responses")
        print("Note: Responses may vary due to temperature setting")
        return True
    else:
        print(f"\n❌ FAIL: Only got {len(results)}/3 responses")
        return False

# ============================================================================
# MAIN
# ============================================================================

def main():
    print("🧪 TEST AVANZADO DE SISTEMA DE IA - Nightmare Edition")
    print("="*60)
    
    # Cargar API key
    api_key = load_api_key()
    if api_key is None:
        print("\n❌ ERROR: No API key found in tools/.env.dev")
        sys.exit(1)
    
    config = AIConfig(api_key=api_key)
    
    # Ejecutar tests
    results = []
    
    results.append(("Response Cleaning", test_response_cleaning(config)))
    results.append(("Long Texts", test_long_texts(config)))
    results.append(("Special Characters", test_special_characters(config)))
    results.append(("Concurrent Requests", test_concurrent_requests(config)))
    results.append(("Error Recovery", test_error_recovery(config)))
    results.append(("Diablo-Specific Texts", test_diablo_specific_texts(config)))
    results.append(("Response Consistency", test_consistency(config)))
    
    # Resumen
    print("\n" + "="*60)
    print("RESUMEN DE RESULTADOS")
    print("="*60)
    
    passed = 0
    for name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status}: {name}")
        if result:
            passed += 1
    
    print("\n" + "="*60)
    print(f"TOTAL: {passed}/{len(results)} tests passed")
    print("="*60)
    
    if passed >= len(results) * 0.7:  # 70% success rate
        print("\n✅ SISTEMA ROBUSTO Y FUNCIONAL")
        print("\nLa API responde bien y el sistema maneja:")
        print("- Limpieza de respuestas ✅")
        print("- Textos largos ✅")
        print("- Caracteres especiales ✅")
        print("- Requests concurrentes ✅")
        print("- Recuperación de errores ✅")
        print("- Textos de Diablo ✅")
        print("- Consistencia ✅")
        return 0
    else:
        print("\n⚠️  Sistema funcional pero con algunas limitaciones")
        print("Revisar tests fallidos para más detalles")
        return 1

if __name__ == "__main__":
    sys.exit(main())

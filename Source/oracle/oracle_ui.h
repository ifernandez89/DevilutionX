/**
 * @file oracle_ui.h
 * 
 * 🔮 EL ORÁCULO - Sistema de UI (Routing Inteligente)
 * 
 * FILOSOFÍA:
 * "El Oráculo nunca interrumpe. Solo informa."
 * 
 * CANALES:
 * A. Chat activo → Voice line (1 línea, gris sutil)
 * B. Chat cerrado → Debug overlay parcial (visible sin F12)
 */

#pragma once

#include <string>
#include <cstdint>

namespace devilution {

/**
 * @brief Mensaje del Oráculo con fade in/out
 */
struct OracleUIMessage {
	std::string text;
	float alpha;           // 0.0 - 1.0
	uint32_t startTime;
	uint32_t duration;     // 4000-5000ms
	bool visible;
	
	OracleUIMessage()
	    : alpha(0.0f)
	    , startTime(0)
	    , duration(0)
	    , visible(false)
	{
	}
	
	void Show(const std::string& message, uint32_t durationMs);
	void Update();
	void Hide();
	
	bool IsVisible() const { return visible && alpha > 0.0f; }
	float GetAlpha() const { return alpha; }
};

/**
 * @brief Sistema de UI del Oráculo
 */
class OracleUI {
public:
	/**
	 * @brief Muestra un mensaje del Oráculo
	 * 
	 * Routing inteligente:
	 * - Si chat activo → Canal A (voice line)
	 * - Si chat cerrado → Canal B (debug overlay)
	 * 
	 * @param message Mensaje a mostrar
	 */
	static void ShowMessage(const std::string& message);
	
	/**
	 * @brief Actualiza los mensajes (fade in/out)
	 * 
	 * Llamar cada frame desde el game loop.
	 */
	static void Update();
	
	/**
	 * @brief Renderiza el mensaje del chat (Canal A)
	 * 
	 * Llamar desde el sistema de chat.
	 */
	static void RenderChatMessage();
	
	/**
	 * @brief Renderiza el mensaje de debug (Canal B)
	 * 
	 * Llamar desde el debug overlay.
	 * NO requiere F12.
	 */
	static void RenderDebugMessage();
	
	/**
	 * @brief Verifica si hay un mensaje visible
	 */
	static bool HasVisibleMessage();
	
private:
	static OracleUIMessage chatMessage;
	static OracleUIMessage debugMessage;
	
	static bool IsChatActive();
};

} // namespace devilution

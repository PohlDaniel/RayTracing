#include "Model.h"
#include "KDTree.h"

Model::Model() :OrientablePrimitive() {

	m_hasMaterial = false;
	m_hasNormals = false;
	m_hasTangents = false;
	m_color = Color(1.0, 1.0, 0.0);
	m_texture = NULL;
	m_material = NULL;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}

Model::Model(const Color &color) :OrientablePrimitive(color) {

	m_hasMaterial = false;
	m_hasNormals = false;
	m_hasTangents = false;
	m_color = color;
	m_texture = NULL;
	m_material = NULL;

	xmin = FLT_MAX;
	ymin = FLT_MAX;
	zmin = FLT_MAX;
	xmax = -FLT_MAX;
	ymax = -FLT_MAX;
	zmax = -FLT_MAX;
}



Model::~Model(){

	
}


void Model::hit(const Ray& a_ray, Hit &hit){

	// find the nearest intersection
	m_KDTree->intersectRec(a_ray, hit);

}

bool Model::loadObject(const char* filename, bool cull){

	return loadObject(filename, Vector3f(1.0, 0.0, 0.0), 0.0, Vector3f(0.0, 0.0, 0.0), 1.0, cull);
}

void Model::calcBounds(){

	Vector3f p1 = Vector3f(xmin, ymin, zmin);
	Vector3f p2 = Vector3f(xmax, ymax, zmax);

	box = BBox(p1, p2 - p1);
	Model::bounds = true;
}

Color Model::getColor(const Vector3f& a_pos){
	
	

	if (m_texture){
		
		m_KDTree->m_primitive->m_texture = m_texture;
		return m_KDTree->m_primitive->getColor(a_pos);


	}else if (m_KDTree->m_primitive->m_texture){

		
		return m_KDTree->m_primitive->getColor(a_pos);


	}else {

		return m_KDTree->m_primitive->m_color;
	}

	

	
}

Vector3f  Model::getNormal(const Vector3f& a_pos){
	
	if (m_hasNormals){

		return (invT *  m_KDTree->m_primitive->getNormal(a_pos)).normalize();
	}
	
	
	return Vector3f(0.0, 0.0, 0.0);
	
}

std::shared_ptr<Material> Model::getMaterial(){

	if (m_material){

		return m_material;

	}else{

		return m_KDTree->m_primitive->m_material;
	}
}

bool compare(const std::array<int, 10> &i_lhs, const std::array<int, 10> &i_rhs){

	return i_lhs[9] < i_rhs[9];
}

bool Model::loadObject(const char* a_filename, Vector3f &axis, float degree, Vector3f &translate, float scale, bool cull){

	std::string filename(a_filename);

	const size_t index = filename.rfind('/');

	if (std::string::npos != index){

		m_modelDirectory = filename.substr(0, index);
	}

	
	std::vector<std::array<int, 10>> face;

	std::vector<Vector3f> positionCoords;
	std::vector<Vector3f> normalCoords;
	std::vector<Vector2f> textureCoords;

	std::map<std::string, int> name;


	int countMesh = 0;
	int assign = 0;
	char buffer[250];


	FILE * pFile = fopen(a_filename, "r");
	if (pFile == NULL){
		std::cout << "File not found" << std::endl;
		return false;
	}

	Matrix4f rotMtx;
	rotMtx.rotate(axis, degree);

	int tmp = 0;

	while (fscanf(pFile, "%s", buffer) != EOF){
		
		switch (buffer[0]){

			case '#':{
					 fgets(buffer, sizeof(buffer), pFile);
				     break;

			}case 'm':{
					 fgets(buffer, sizeof(buffer), pFile);
					 sscanf(buffer, "%s %s", buffer, buffer);
					 m_mltPath = buffer;

					 m_hasMaterial = true;
					 break;

			}case 'v':{
				
					switch (buffer[1]){

						 case '\0':{
									 
							 float tmpx, tmpy, tmpz;
							 fgets(buffer, sizeof(buffer), pFile);
							 sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

							 Vector3f position = Vector3f(tmpx, tmpy, tmpz);
							 position = rotMtx * position;
							 positionCoords.push_back(Vector3f(position[0] * scale, position[1] * scale, position[2] * scale));
									  
							 break;

						 }case 't':{

							 float tmpu, tmpv;
							 fgets(buffer, sizeof(buffer), pFile);
							 sscanf(buffer, "%f %f", &tmpu, &tmpv);
							 textureCoords.push_back(Vector2f(tmpu, tmpv));
							 break;

						 }case 'n':{

							 float tmpx, tmpy, tmpz;
							 fgets(buffer, sizeof(buffer), pFile);
							 sscanf(buffer, "%f %f %f", &tmpx, &tmpy, &tmpz);

							 Vector3f normal = Vector3f(tmpx, tmpy, tmpz);
							 Matrix4f rotMtx;
							 rotMtx.rotate(axis, degree);
							 normal = rotMtx * normal;
							 normalCoords.push_back(Vector3f(normal[0], normal[1], normal[2]));
							 break;

						 }default:{

							 break;
						 }
				 }
				 break;

			}case 'u': {
				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%s %s", buffer, buffer);

				std::map<std::string, int >::const_iterator iter = name.find(buffer);

				if (iter == name.end()){
					// mlt name not found
					countMesh++;
					assign = countMesh;

					name[buffer] = countMesh;


				}
				else{
					// mlt name found
					assign = iter->second;
				}

				break;

			}case 'g': {
				fgets(buffer, sizeof(buffer), pFile);
				sscanf(buffer, "%s", buffer);

				countMesh++;
				assign = countMesh;
				name[buffer] = countMesh;
				break;

			}case 'f': {
				
				int a, b, c, n1, n2, n3, t1, t2, t3;
				fgets(buffer, sizeof(buffer), pFile);

				if (!textureCoords.empty() && !normalCoords.empty()){
					sscanf(buffer, "%d/%d/%d %d/%d/%d %d/%d/%d ", &a, &t1, &n1, &b, &t2, &n2, &c, &t3, &n3);
					face.push_back({ { a, b, c, t1, t2, t3, n1, n2, n3, assign } });

				}
				else if (!normalCoords.empty()){
					sscanf(buffer, "%d//%d %d//%d %d//%d", &a, &n1, &b, &n2, &c, &n3);
					face.push_back({ { a, b, c, 0, 0, 0, n1, n2, n3, assign } });

				}
				else if (!textureCoords.empty()){
					sscanf(buffer, "%d/%d %d/%d %d/%d", &a, &t1, &b, &t2, &c, &t3);
					face.push_back({ { a, b, c, t1, t2, t3, 0, 0, 0, assign } });

				}
				else {
					sscanf(buffer, "%d %d %d", &a, &b, &c);
					face.push_back({ { a, b, c, 0, 0, 0, 0, 0, 0, assign } });
				}
				break;

			}default: {

				break;
			}
		}

	}

	

	std::sort(face.begin(), face.end(), compare);

	std::map<int, int> dup;

	for (unsigned int i = 0; i < face.size(); i++){
		dup[face[i][9]]++;
	}

	

	m_numberOfMeshes = dup.size();

	std::map<int, int>::const_iterator iterDup = dup.begin();

	for (iterDup; iterDup != dup.end(); iterDup++){


		if (name.empty()){

			meshes.push_back(std::shared_ptr<Mesh>(new Mesh(iterDup->second)));

		}
		else{

			std::map<std::string, int >::const_iterator iterName = name.begin();
			for (iterName; iterName != name.end(); iterName++){

				if (iterDup->first == iterName->second){



					meshes.push_back(std::shared_ptr<Mesh>(new Mesh("newmtl " + iterName->first, iterDup->second)));
				}
			}

		}
	}

	dup.clear();
	name.clear();

	for (int j = 0; j < m_numberOfMeshes; j++){
		
		
		
		if (meshes[j]->readMaterial((m_modelDirectory + "/" + m_mltPath).c_str())){
			
			if (meshes[j]->m_material->colorMapPath != ""){

				meshes[j]->m_texture = std::shared_ptr<Texture>(new Texture(&(m_modelDirectory + "/" + meshes[j]->m_material->colorMapPath)[0]));
			}

		}else{

			meshes[j]->m_color = Color(1.0f / (j + 1), 1.0f / (j + 1), 1.0f / (j + 1));

			if (m_material){

				meshes[j]->m_material = std::shared_ptr<Material> (m_material);

			}else{
				meshes[j]->m_material = std::shared_ptr<Material> (new Material());

			}

		}

		
	}
	


	
	int start = 0;
	int end = meshes[0]->m_numberTriangles;

	

	for (int j = 0; j < m_numberOfMeshes; j++){

		
		if (j > 0){

			start = end;
			end = end + meshes[j]->m_numberTriangles;
		}

		Vector3f a;
		Vector3f b;
		Vector3f c;
		std::shared_ptr<Triangle> triangle;


		

		for (int i = start; i < end; i++){

			a = positionCoords[(face[i])[0] - 1];
			b = positionCoords[(face[i])[1] - 1];
			c = positionCoords[(face[i])[2] - 1];

			meshes[j]->m_numberOfVertices = max((face[i])[0], max((face[i])[1], max((face[i])[2], meshes[j]->m_numberOfVertices)));

			meshes[j]->m_indexBuffer.push_back((face[i])[0] - 1);
			meshes[j]->m_indexBuffer.push_back((face[i])[1] - 1);
			meshes[j]->m_indexBuffer.push_back((face[i])[2] - 1);

			

			meshes[j]->m_xmin = min(a[0] + translate[0], min(b[0] + translate[0], min(c[0] + translate[0], meshes[j]->m_xmin)));
			meshes[j]->m_ymin = min(a[1] + translate[1], min(b[1] + translate[1], min(c[1] + translate[1], meshes[j]->m_ymin)));
			meshes[j]->m_zmin = min(a[2] + translate[2], min(b[2] + translate[2], min(c[2] + translate[2], meshes[j]->m_zmin)));

			meshes[j]->m_xmax = max(a[0] + translate[0], max(b[0] + translate[0], max(c[0] + translate[0], meshes[j]->m_xmax)));
			meshes[j]->m_ymax = max(a[1] + translate[1], max(b[1] + translate[1], max(c[1] + translate[1], meshes[j]->m_ymax)));
			meshes[j]->m_zmax = max(a[2] + translate[2], max(b[2] + translate[2], max(c[2] + translate[2], meshes[j]->m_zmax)));

			triangle = std::shared_ptr<Triangle>(new Triangle(a + translate, b + translate, c + translate, Color(0.6f, 1.0f, 1.0f), cull));
			triangle->m_texture = meshes[j]->m_texture;
			triangle->m_material = meshes[j]->m_material;
			triangle->m_color = meshes[j]->m_color;
			
			if (textureCoords.size() > 0){

				meshes[j]->m_numberOfTexels = max((face[i])[3], max((face[i])[4], max((face[i])[5], meshes[j]->m_numberOfTexels)));
				meshes[j]->m_indexBufferTexel.push_back((face[i])[3] - 1);
				meshes[j]->m_indexBufferTexel.push_back((face[i])[4] - 1);
				meshes[j]->m_indexBufferTexel.push_back((face[i])[5] - 1);

				meshes[j]->m_hasTexels = true;
				
				triangle->setUV(textureCoords[(face[i])[3] - 1], textureCoords[(face[i])[4] - 1], textureCoords[(face[i])[5] - 1] );
			}


			if (normalCoords.size() > 0){
				
				meshes[j]->m_numberOfNormals = max((face[i])[6], max((face[i])[7], max((face[i])[8], meshes[j]->m_numberOfNormals)));
				meshes[j]->m_indexBufferNormal.push_back((face[i])[6] - 1);
				meshes[j]->m_indexBufferNormal.push_back((face[i])[7] - 1);
				meshes[j]->m_indexBufferNormal.push_back((face[i])[8] - 1);

				meshes[j]->m_hasNormals = true;
				m_hasNormals = true;

				triangle->setNormal(normalCoords[(face[i])[6] - 1], normalCoords[(face[i])[7] - 1], normalCoords[(face[i])[8] - 1]);
			}

			meshes[j]->m_triangles.push_back(triangle);
		}

		std::vector<Vector3f>::const_iterator first;
		std::vector<Vector3f>::const_iterator last;

		if (j > 0){
			first = positionCoords.begin() + meshes[j - 1]->m_numberOfVertices;
			last = positionCoords.begin() + meshes[j - 1]->m_numberOfVertices + meshes[j]->m_numberOfVertices;
		
		}else{

			first = positionCoords.begin();
			last = positionCoords.begin() + meshes[j]->m_numberOfVertices;
		}

		meshes[j]->m_positions = std::vector<Vector3f>(first, last);

		if (!normalCoords.empty()){

			if (j > 0){
				first = normalCoords.begin() + meshes[j - 1]->m_numberOfNormals;
				last = normalCoords.begin() + meshes[j - 1]->m_numberOfNormals + meshes[j]->m_numberOfNormals;

			}else{

				first = normalCoords.begin();
				last = normalCoords.begin() + meshes[j]->m_numberOfNormals;
			}

			meshes[j]->m_normals = std::vector<Vector3f>(first, last);
		}

		if (!textureCoords.empty()){

			std::vector<Vector2f>::const_iterator firstTexture;
			std::vector<Vector2f>::const_iterator lastTexture;

			if (j > 0){
				firstTexture = textureCoords.begin() + meshes[j - 1]->m_numberOfTexels;
				lastTexture = textureCoords.begin() + meshes[j - 1]->m_numberOfTexels + meshes[j]->m_numberOfTexels;

			}else{

				firstTexture = textureCoords.begin();
				lastTexture = textureCoords.begin() + meshes[j]->m_numberOfTexels;
			}

			meshes[j]->m_texels = std::vector<Vector2f>(firstTexture, lastTexture);
		}


		xmin = min(meshes[j]->m_xmin, xmin);
		ymin = min(meshes[j]->m_ymin, ymin);
		zmin = min(meshes[j]->m_zmin, zmin);

		xmax = max(meshes[j]->m_xmax, xmax);
		ymax = max(meshes[j]->m_ymax, ymax);
		zmax = max(meshes[j]->m_zmax, zmax);
		
		
	}

	

	std::cout << "Number of faces: " << face.size() << std::endl;
	calcBounds();
	



	
	return true;
}

void Model::buildKDTree(){

	std::cout << "Build KDTree!" << std::endl;


	for (int j = 0; j < m_numberOfMeshes; j++){



		m_triangles.insert(m_triangles.end(), meshes[j]->m_triangles.begin(), meshes[j]->m_triangles.end());
		meshes[j]->m_triangles.clear();


	}

	/*for (int i = 0; i < m_triangles.size(); i++){
		std::cout << m_triangles[i]->m_texture << std::endl;
	}*/

	m_KDTree = std::unique_ptr<KDTree>(new KDTree());
	m_KDTree->buildTree(m_triangles, box);

	std::cout << "Finished KDTree!" << std::endl;

}

void Model::generateNormals(){

	for (int j = 0; j < m_numberOfMeshes; j++){
		meshes[j]->generateNormals();
	}

	m_hasNormals = true;
}

void Model::generateTangents(){

	for (int j = 0; j < m_numberOfMeshes; j++){
		meshes[j]->generateTangents();
	}
	m_hasNormals = true;
	m_hasTangents = true;
}

////////////////////////////////////////////////////Mesh////////////////////////////////////

Mesh::Mesh(std::string mltName, int numberTriangles){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberTriangles = numberTriangles;
	m_mltName = mltName;
	m_texture = NULL;
	m_material = NULL;

	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;

	m_numberOfVertices = 0;
	m_numberOfTexels = 0;
	m_numberOfNormals = 0;

	m_indexBuffer.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();

	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

Mesh::Mesh(int numberTriangles){

	m_color = Color(1.0, 1.0, 1.0);
	m_numberTriangles = numberTriangles;
	m_texture = NULL;
	m_material = NULL;

	m_hasNormals = false;
	m_hasTexels = false;
	m_hasTangents = false;

	m_numberOfVertices = 0;
	m_numberOfTexels = 0;
	m_numberOfNormals = 0;

	m_indexBuffer.clear();
	m_indexBufferTexel.clear();
	m_indexBufferNormal.clear();

	m_xmin = FLT_MAX;
	m_ymin = FLT_MAX;
	m_zmin = FLT_MAX;
	m_xmax = -FLT_MAX;
	m_ymax = -FLT_MAX;
	m_zmax = -FLT_MAX;
}

Mesh::~Mesh(){


}


void Mesh::generateNormals(){

	if (m_hasNormals) { return; }

	Vector3f normal;

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f normal0;
	Vector3f normal1;
	Vector3f normal2;

	Vector3f edge1;
	Vector3f edge2;

	const unsigned int *pTriangle = 0;
	
	for (unsigned int i = 0; i < m_positions.size(); i++){

		m_normals.push_back(Vector3f(0.0, 0.0, 0.0));
		
	}

	for (unsigned int i = 0; i < m_numberTriangles; i++){

		
		pTriangle = &m_indexBuffer[i * 3];

		vertex0 = m_positions[pTriangle[0]];
		vertex1 = m_positions[pTriangle[1]];
		vertex2 = m_positions[pTriangle[2]];

		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		normal = Vector3f::cross(edge1, edge2);

		

		m_normals[pTriangle[0]] = m_normals[pTriangle[0]] + normal;
		m_normals[pTriangle[1]] = m_normals[pTriangle[1]] + normal;
		m_normals[pTriangle[2]] = m_normals[pTriangle[2]] + normal;

	}

	
	for (unsigned int i = 0; i < m_numberTriangles; i++){
	
		pTriangle = &m_indexBuffer[i * 3];

		Vector3f::normalize(m_normals[pTriangle[0]]);
		Vector3f::normalize(m_normals[pTriangle[1]]);
		Vector3f::normalize(m_normals[pTriangle[2]]);

		m_triangles[i]->setNormal(m_normals[pTriangle[0]], m_normals[pTriangle[1]], m_normals[pTriangle[2]]);
		
	}

	m_indexBufferNormal.clear();
	m_hasNormals = true;

}

void Mesh::generateTangents(){

	if (m_hasTangents){ std::cout << "Tangents already generate!" << std::endl; return; }
	if (!m_hasTexels){ std::cout << "TextureCoords needed!" << std::endl; return; }
	if (!m_hasNormals){
		generateNormals();
		std::cout << "Normals generated, they needed to calculate the bitangents and setup the TNB-matrix!" << std::endl;

	}

	

	Vector3f vertex0;
	Vector3f vertex1;
	Vector3f vertex2;

	Vector3f edge1;
	Vector3f edge2;

	Vector2f texEdge1;
	Vector2f texEdge2;

	Vector3f normal;
	Vector4f  tangent;
	Vector3f  bitangent;

	float det = 0.0f;
	float nDotT = 0.0f;
	float length = 0.0f;
	float bDotB = 0.0f;

	const unsigned int *pTriangle = 0;
	const unsigned int *pTriangleTex = 0;
	const unsigned int *pTriangleNormal = 0;

	std::vector<Vector3f> tmpNormals;

	for (int i = 0; i < m_positions.size(); i++){

		m_tangents.push_back(Vector4f(0.0, 0.0, 0.0, 0.0));
		

		m_bitangents.push_back(Vector3f(0.0, 0.0, 0.0));
		

		if (!m_indexBufferNormal.empty()){

			tmpNormals.push_back(Vector3f(0.0, 0.0, 0.0));

		}
	}


	// Calculate the vertex tangents and bitangents.
	for (int i = 0; i < m_numberTriangles; i++){

		pTriangle = &m_indexBuffer[i * 3];
		pTriangleTex = &m_indexBufferTexel[i * 3];

		vertex0 = m_positions[pTriangle[0]];
		vertex1 = m_positions[pTriangle[1]];
		vertex2 = m_positions[pTriangle[2]];

		// Calculate the triangle face tangent and bitangent.

		edge1 = vertex1 - vertex0;
		edge2 = vertex2 - vertex0;

		texEdge1 = m_texels[pTriangleTex[1]] - m_texels[pTriangleTex[0]];
		texEdge2 = m_texels[pTriangleTex[2]] - m_texels[pTriangleTex[0]];

		det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

		if (fabs(det) < 1e-6f){

			tangent[0] = 1.0f;
			tangent[1] = 0.0f;
			tangent[2] = 0.0f;

			bitangent[0] = 0.0f;
			bitangent[1] = 1.0f;
			bitangent[2] = 0.0f;

		}else{

			det = 1.0f / det;

		
			tangent[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
			tangent[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[1]) * det;
			tangent[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[2]) * det;
			tangent[3] = 0.0f;

			bitangent = (edge2 * texEdge1[0] - edge1 * texEdge2[0]) * det;

		}

	

		// Accumulate the tangents and bitangents.
		m_tangents[pTriangle[0]] = m_tangents[pTriangle[0]] + tangent;
		m_tangents[pTriangle[1]] = m_tangents[pTriangle[1]] + tangent;
		m_tangents[pTriangle[2]] = m_tangents[pTriangle[2]] + tangent;

		m_bitangents[pTriangle[0]] = m_bitangents[pTriangle[0]] + bitangent;
		m_bitangents[pTriangle[1]] = m_bitangents[pTriangle[1]] + bitangent;
		m_bitangents[pTriangle[2]] = m_bitangents[pTriangle[2]] + bitangent;


		// Order the normals
		if (!m_indexBufferNormal.empty()){

			pTriangleNormal = &m_indexBufferNormal[i * 3];

			if (tmpNormals[pTriangle[0]].null()){

				tmpNormals[pTriangle[0]] = m_normals[pTriangleNormal[0]];
			}

			if (tmpNormals[pTriangle[1]].null()){

				tmpNormals[pTriangle[1]] = m_normals[pTriangleNormal[1]];
			}

			if (tmpNormals[pTriangle[1]].null()){

				tmpNormals[pTriangle[1]] = m_normals[pTriangleNormal[1]];
			}
		}
	}


	

	if (!m_indexBufferNormal.empty()){

		m_normals.clear();
		m_normals.swap(tmpNormals);
	}
	// Orthogonalize and normalize the vertex tangents.
	for (unsigned int i = 0; i < m_positions.size(); i++){

		
		// Gram-Schmidt orthogonalize tangent with normal.

		nDotT = m_normals[i][0] * m_tangents[i][0] +
			m_normals[i][1] * m_tangents[i][1] +
			m_normals[i][2] * m_tangents[i][2];

		m_tangents[i][0] -= m_normals[i][0] * nDotT;
		m_tangents[i][1] -= m_normals[i][1] * nDotT;
		m_tangents[i][2] -= m_normals[i][2] * nDotT;

		// Normalize the tangent.
		Vector4f::normalize(m_tangents[i]);
		
		
		
		// Calculate the handedness of the local tangent space.
		// The bitangent vector is the cross product between the triangle face
		// normal vector and the calculated tangent vector. The resulting
		// bitangent vector should be the same as the bitangent vector
		// calculated from the set of linear equations above. If they point in
		// different directions then we need to invert the cross product
		// calculated bitangent vector. We store this scalar multiplier in the
		// tangent vector's 'w' component so that the correct bitangent vector
		// can be generated in the normal mapping shader's vertex shader.
		//
		// Normal maps have a left handed coordinate system with the origin
		// located at the top left of the normal map texture. The x coordinates
		// run horizontally from left to right. The y coordinates run
		// vertically from top to bottom. The z coordinates run out of the
		// normal map texture towards the viewer. Our handedness calculations
		// must take this fact into account as well so that the normal mapping
		// shader's vertex shader will generate the correct bitangent vectors.
		// Some normal map authoring tools such as Crazybump
		// (http://www.crazybump.com/) includes options to allow you to control
		// the orientation of the normal map normal's y-axis.

		bitangent[0] = (m_normals[i][1] * m_tangents[i][2]) -
			(m_normals[i][2] * m_tangents[i][1]);
		bitangent[1] = (m_normals[i][2] * m_tangents[i][0]) -
			(m_normals[i][0] * m_tangents[i][2]);
		bitangent[2] = (m_normals[i][0] * m_tangents[i][1]) -
			(m_normals[i][1] * m_tangents[i][0]);

		bDotB = bitangent[0] * m_bitangents[i][0] +
			bitangent[1] * m_bitangents[i][1] +
			bitangent[2] * m_bitangents[i][2];

		// Calculate handedness
		m_tangents[i][3] = (bDotB < 0.0f) ? 1.0f : -1.0f;
		m_bitangents[i] = bitangent;
		
	}

	for (unsigned int i = 0; i < m_numberTriangles; i++){

		pTriangle = &m_indexBuffer[i * 3];

		m_triangles[i]->setTangents(m_tangents[pTriangle[0]], m_tangents[pTriangle[1]], m_tangents[pTriangle[2]]);
		m_triangles[i]->setBiTangents(m_bitangents[pTriangle[0]], m_bitangents[pTriangle[1]], m_bitangents[pTriangle[2]]);
	}

	m_hasTangents = true;


}

bool Mesh::readMaterial(const char* filename){

	m_material = std::shared_ptr<Material>(new Material());
	std::vector<std::string*>lines;
	int start = -1;
	int end = -1;

	std::ifstream in(filename);


	if (!in.is_open()){

		std::cout << "Mlt file not found" << std::endl;
		return false;
	}

	std::string line;
	while (getline(in, line)){
		lines.push_back(new std::string(line));

	}
	in.close();


	for (unsigned int i = 0; i < lines.size(); i++){

		if (strcmp((*lines[i]).c_str(), m_mltName.c_str()) == 0){

			start = i;
			continue;
		}


		if ((*lines[i]).find("newmtl") != std::string::npos && start > 0){

			end = i;
			break;
		}
		else {

			end = lines.size();
		}

	}

	if (start < 0 || end < 0) return false;

	for (int i = start; i < end; i++){

		if ((*lines[i])[0] == '#'){

			continue;

		}
		else if ((*lines[i])[0] == 'N' && (*lines[i])[1] == 's'){
			
			int tmp;
			sscanf(lines[i]->c_str(), "Ns %i", &tmp);

			m_material->m_shinies = tmp;
			

		}else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'a'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ka %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_ambient = Color(tmpx, tmpy, tmpz);
		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 'd'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Kd %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_diffuse =  Color(tmpx, tmpy, tmpz);

		}
		else if ((*lines[i])[0] == 'K' && (*lines[i])[1] == 's'){
			float tmpx, tmpy, tmpz;
			sscanf(lines[i]->c_str(), "Ks %f %f %f", &tmpx, &tmpy, &tmpz);

			

			m_material->m_specular =  Color(tmpx, tmpy, tmpz);

		}else if ((*lines[i])[0] == 'm'){


			char identifierBuffer[20], valueBuffer[250];;
			sscanf(lines[i]->c_str(), "%s %s", identifierBuffer, valueBuffer);

			if (strstr(identifierBuffer, "map_Kd") != 0){

				m_material->colorMapPath = valueBuffer;

				
			}
			else if (strstr(identifierBuffer, "map_bump") != 0){

				m_material->bumpMapPath = valueBuffer;

			}

		}
	}



	for (unsigned int i = 0; i < lines.size(); i++){

		delete lines[i];
	}

	return true;
}
